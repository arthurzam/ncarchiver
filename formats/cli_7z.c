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

static const char *_7z_cmds[] = {"7z", "7za", "7zr", NULL};
static const uint8_t _7z_cmds_uses[] = {CLI_ALL, CLI_ALL, CLI_ALL};
static const char *_7z_addSwitch[] = {"a", "-l", NULL};
static const char *_7z_extractSwitch[] = {"x", NULL};
static const char *_7z_delSwitch[] = {"d", "-bd", NULL};
static const char *_7z_listSwitch[] = {"l", "-slt", "-bd", NULL};
static const char *_7z_passwordSwitch[] = {"-p%s", NULL};
static const char *_7z_testSwitch[] = {"t", "-bd", NULL};

static const char *_7z_errorWrongPassword[] = {"Wrong password", NULL};
static const char *_7z_errorCorruptedArchive[] = {"Unexpected end of archive", "Headers Error", NULL};
static const char *_7z_errorFullDisk[] = {"No space left on device", NULL};
static const char *_7z_fileExistsPatterns[] = {
    "\\(Y\\)es / \\(N\\)o / \\(A\\)lways / \\(S\\)kip all / A\\(u\\)to rename all / \\(Q\\)uit",
    NULL};
static const char *_7z_fileExistsFileName[] = {"^file \\./(.*)$", "^  Path:     \\./(.*)$", NULL};

static const char *_7z_mimes[] = {
    "application/x-7z-compressed",
    "application/zip",
    NULL
};

//static struct tm *mktime_from_string (char *str) // YY-MM-DD hh:mm:ss
//{
//    static struct tm tm = {0, };

//    tm.tm_isdst = -1;

//    sscanf(str, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

//    tm.tm_year -= 1900;
//    tm.tm_mon  -= 1;

//    return &tm;
//}

static struct dir_t *cli_7z_processList(struct archive_t *archive, FILE *inF, FILE *outF)
{
    (void)archive;
    (void)inF;
    int state = 0;
    struct dir_t *root = NULL, *temp;
    regex_t re;
    regcomp(&re, "^p7zip Version ([0-9\\.]+) ", REG_EXTENDED);
    regmatch_t matches[10];
    int i;
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
                i = regexec(&re, line, sizeof(matches) / sizeof(matches[0]), (regmatch_t *)&matches, 0);
                if (i == REG_NOERROR)
                {
                    state = 1;
                    line[matches[1].rm_eo] = '\0';
                    regfree(&re);
                    LOG_I("7z", "p7zip version %s detected", line + matches[1].rm_so);
                }
                break;
            case 1: // parse header
                if (strstartswith(line, "Listing archive:"))
                    LOG_I("7z", "listing archive: %s", line + 16);
                else if (0 == strcmp(line, "--") || 0 == strcmp(line, "----"))
                    state = 2;
                else if (strstartswith(line, "Enter password (will not be echoed)"))
                {
                    archive->flags |= ARCHIVE_ENCRYPTED;
                    archive->error = ARCHIVE_ERROR_BAD_PASSWORD;
                    LOG_e("7z", "missing password");
                    return NULL;
                }
                else if (NULL != strstr(line, "Error: "))
                {
                    LOG_E("7z", "error parsing header: %s", line + 7);
                    return NULL;
                }
                break;
            case 2: // parse archive information
                if (0 == strcmp(line, "----------"))
                    state = 4;
                else if (strstartswith(line, "Type = "))
                    LOG_I("7z", "type: %s", line + 7);
                else if (strstartswith(line, "Method = "))
                    LOG_I("7z", "method: %s", line + 9);
                else if (strstartswith(line, "Comment = "))
                {
                    arr_len = strlen(line + 10);
                    archive->comment = (char *)malloc(arr_size = arr_len + arr_len / 2);
                    memcpy(archive->comment, line + 10, arr_len);
                    archive->comment[arr_len] = '\n';
                    archive->comment[++arr_len] = '\0';
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
                        archive->comment = realloc(archive->comment, arr_size += (addlen + 10));
                    strcat(archive->comment, line);
                    strcat(archive->comment, "\n");
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
                    temp->moreInfo[arr_len].value = strdup(line + 6);
                    temp->moreInfo[++arr_len].key = NULL;
                }
                else if (strstartswith(line, "Encrypted = "))
                {
                    if (0 == strcmp(line + 12, "+"))
                        archive->flags |= ARCHIVE_ENCRYPTED;
                }
                else if (strstartswith(line, "Modified = "))
                {
                    if (line[11] == '\0') break;
                    if (arr_size < arr_len + 1)
                        temp->moreInfo = (struct dir_more_info_t *)realloc(temp->moreInfo, sizeof(struct dir_more_info_t) * (arr_size += 3));
                    temp->moreInfo[arr_len].key = "Modified";
                    temp->moreInfo[arr_len].value = strdup(line + 11);
                    temp->moreInfo[++arr_len].key = NULL;

                    //    Link: https://stackoverflow.com/a/3054052
                }
                break;
        }
    }

    return root;
}

static const struct cli_format_t cli_7z_proc = {
    .parent = {
        .name = "cli 7z",
        .mime_types_rw = _7z_mimes,
        .openArchive = format_default_openArchive,
        .listFiles = cli_listFiles,
        .extractFiles = cli_extractFiles,
        .deleteFiles = cli_deleteFiles,
        .testFiles = cli_testFiles,
        .closeArchive = format_default_closeArchive
    },
    .cmds = _7z_cmds,
    .cmds_uses = _7z_cmds_uses,

    .addSwitch = _7z_addSwitch,
    .extractSwitch = _7z_extractSwitch,
    .delSwitch = _7z_delSwitch,
    .listSwitch = _7z_listSwitch,
    .passwordSwitch = _7z_passwordSwitch,
    .testSwitch = _7z_testSwitch,

    .errorWrongPassword = _7z_errorWrongPassword,
    .errorCorruptedArchive = _7z_errorCorruptedArchive,
    .errorFullDisk = _7z_errorFullDisk,
    .fileExistsPatterns = _7z_fileExistsPatterns,
    .fileExistsFileName = _7z_fileExistsFileName,
    .fileExistsInput = {"Y", "N", "A", "S", "Q"},

    .processList = cli_7z_processList
};

ADD_FORMAT(cli_7z_proc);
