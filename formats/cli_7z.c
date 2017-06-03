#include "global.h"
#include "cli.h"
#include "filetree.h"

#include <archive.h>
#include <archive_entry.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <regex.h>

static char *_7z_cmds[] = {"7z", NULL};
static const uint8_t _7z_cmds_uses[] = {CLI_ALL};
static const char *_7z_addSwitch[] = {"a", "-l", NULL};
static const char *_7z_listSwitch[] = {"l", "-slt", NULL};
static const char *_7z_passwordSwitch[] = {"-p%s", NULL};

static const char *_7z_mimes[] = {
    "application/x-7z-compressed",
    "application/zip",
    NULL
};

static time_t mktime_from_string (char *str)
{
    struct tm tm = {0, };
    char *s = str, *e;

    tm.tm_isdst = -1;

    /* date */
    *(e = strchr(s, '-')) = '\0';
    tm.tm_year = atoi(s) - 1900;

    *(e = strchr(s = e + 1, '-')) = '\0';
    tm.tm_mon = atoi(s) - 1;

    *(e = strchr(s = e + 1, ' ')) = '\0';
    tm.tm_mday = atoi(s);

    /* time */
    if ((e = strchr(s = e + 1, ':')))
        *e = '\0';
    tm.tm_hour = atoi(s);
    if (e)
    {
        if ((e = strchr(s = e + 1, ':')))
            *e = '\0';
        tm.tm_min = atoi(s);
        if (e)
            tm.tm_sec = atoi(e + 1);
    }

    return mktime(&tm);
}

static struct dir_t *cli_7z_processList(struct archive_t *archive, FILE *inF, FILE *outF)
{
    (void)archive;
    (void)inF;
    int state = 0;
    struct dir_t *root = NULL, *temp;
    regex_t re;
    regmatch_t matches[10];
    int i;
    char *comment = NULL;
    size_t arr_size = 0, arr_len = 0;

    char line[2048 + 1];
    line[sizeof(line) - 1] = '\0';

    while (!feof(outF))
    {
        fgets(line, sizeof(line) - 1, outF);
        line[strlen(line) - 1] = '\0'; // remove new line

        if (*line == '\0')
            continue;

        switch (state)
        {
            case 0: // parse title
                regcomp(&re, "^p7zip Version ([0-9\\.]+) ", REG_EXTENDED);
                i = regexec(&re, line, sizeof(matches) / sizeof(matches[0]), (regmatch_t *)&matches, 0);
                if (i == REG_NOERROR)
                {
                    state = 1;
                    line[matches[1].rm_eo] = '\0';
                    fprintf(stderr, "7z: p7zip version %s detected\n", line + matches[1].rm_so);
                }
                regfree(&re);
                break;
            case 1: // parse header
                if (strstartswith(line, "Listing archive:"))
                    fprintf(stderr, "7z: listing archive: %s\n", line + 16);
                else if (0 == strcmp(line, "--") || 0 == strcmp(line, "----"))
                    state = 2;
                else if (strstartswith(line, "Enter password (will not be echoed)"))
                {
                    archive->flags |= ARCHIVE_ENCRYPTED;
                    archive->error = ARCHIVE_ERROR_BAD_PASSWORD;
                    return NULL;
                }
                else if (NULL != strstr(line, "Error: "))
                {
                    fprintf(stderr, "7z: error parsing header: %s\n", line + 7);
                    return NULL;
                }
                break;
            case 2: // parse archive information
                if (0 == strcmp(line, "----------"))
                    state = 4;
                else if (strstartswith(line, "Type = "))
                    fprintf(stderr, "7z: type: %s\n", line + 7);
                else if (strstartswith(line, "Method = "))
                    fprintf(stderr, "7z: method: %s\n", line + 9);
                else if (strstartswith(line, "Comment = "))
                {
                    arr_len = strlen(line + 10);
                    comment = (char *)malloc(arr_size = arr_len + arr_len / 2);
                    memcpy(comment, line + 10, arr_len);
                    comment[arr_len] = '\n';
                    comment[++arr_len] = '\0';
                    state = 3;
                }
                break;
            case 3: // parse comment
                if (0 == strcmp(line, "----------"))
                    state = 4;
                else
                {
                    size_t addlen = strlen(line);
                    if (arr_len + addlen + 10 >= arr_size)
                        comment = realloc(comment, arr_size += (addlen + 10));
                    strcat(comment, line);
                    strcat(comment, "\n");
                }
                break;
            case 4: // parse entry information
                if (!root)
                    root = filetree_createRoot();

                if (strstartswith(line, "Path = "))
                {
                    temp = filetree_addNode(root, line + 7);
                    arr_size = arr_len = 0;
                }
                else if (strstartswith(line, "Size = "))
                    temp->realSize = atoi(line + 7);
                else if (strstartswith(line, "CRC = "))
                {
                    if (line[6] == '\0') break;
                    if (arr_size < arr_len + 1)
                        temp->moreInfo = (struct dir_more_info_t *)realloc(temp->moreInfo, sizeof(struct dir_more_info_t) * (arr_size += 3));
                    temp->moreInfo[arr_len].key = "CRC";
                    temp->moreInfo[arr_len].value = strdup (line + 6);
                    temp->moreInfo[++arr_len].key = NULL;
                }
                else if (strstartswith(line, "Encrypted = "))
                {
                    if (0 == strcmp(line + 12, "+"))
                        archive->flags |= ARCHIVE_ENCRYPTED;
                }
//                else if (strstartswith(line, "Modified = "))
//                {
                        // Link: https://stackoverflow.com/a/3054052
//                    fprintf(stderr, "7z: file %s modified date %s\n", temp->name, asctime(mktime_from_string(line + 11)));
//                }
                break;

        }
    }
    archive->comment = comment;

    return root;
}

static struct cli_format_t cli_7z_proc = {
    .parent = {
        .name = "cli 7z",
        .mime_types_rw = _7z_mimes,
        .openArchive = format_default_openArchive,
        .listFiles = cli_listFiles,
        .closeArchive = format_default_closeArchive
    },
    .cmds = _7z_cmds,
    .cmds_uses = _7z_cmds_uses,

    .addSwitch = _7z_addSwitch,
    .listSwitch = _7z_listSwitch,
    .passwordSwitch = _7z_passwordSwitch,

    .processList = cli_7z_processList
};

ADD_FORMAT(cli_7z_proc);
