#include "global.h"
#include "cli.h"
#include "filetree.h"

#include <archive.h>
#include <archive_entry.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *zip_cmds[] = {"zip", "unzip", "zipinfo", NULL};
static const uint8_t zip_cmds_uses[] = {CLI_ADD | CLI_DELETE, CLI_EXTRACT | CLI_TEST, CLI_LIST};
static const char *zip_addSwitch[] = {"-r", NULL};
static const char *zip_extractSwitch[] = {"-j", NULL};
static const char *zip_delSwitch[] = {"-d", NULL};
static const char *zip_listSwitch[] = {"-l", "-T", "-z", NULL};
static const char *zip_testSwitch[] = {"-t", NULL};

static const char *zip_passwordSwitch[] = {"-P%s", NULL};
static const struct pMimeStr zip_compressionLevelSwitch[] = {
    {"application/zip", "-%s"},
    {"application/x-java-archive", "-%s"},
    {NULL, NULL}
};
static const struct pMimeStr zip_compressionMethodSwitch[] = {
    {"application/zip", "-Z%s"},
    {"application/x-java-archive", "-Z%s"},
    {NULL, NULL}
};

static struct dir_t *cli_zip_processList(struct archive_t *archive, FILE *inF, FILE *outF)
{
    (void)archive;
    (void)inF;
    int state = 0;
    struct dir_t *root = NULL, *temp;
    size_t arr_size = 0, arr_len = 0;

    char line[2048 + 1];
    line[sizeof(line) - 1] = '\0';

    while (!feof(outF)) {
        fgets(line, sizeof(line) - 1, outF);
        line[strlen(line) - 1] = '\0'; // remove new line

        if (*line == '\0')
            continue;

        switch (state) {
            case 0: // parse header
                if (strstartswith(line, "Archive:  ")) {
                    LOG_I("zip", "listing archive: %s", line + 10);
                    state = 1;
                } else if (strstartswith(line, "Zip file size: "))
                    state = 2;
                break;
            case 1: // parse comment
                if (strstartswith(line, "Zip file size: ")) {
                    state = 2;
                } else {
                    size_t addlen = strlen(line);
                    if (arr_len + addlen + 10 >= arr_size)
                        archive->comment = realloc(archive->comment, arr_size += (addlen + 10));
                    strcat(archive->comment, line);
                    strcat(archive->comment, "\n");
                }
                break;
            case 2: // parse entry information
            {
                if (!root)
                    root = filetree_createRoot();

                // TODO: use sscanf to parse line
                // https://github.com/KDE/ark/blob/master/plugins/clizipplugin/cliplugin.cpp#L158
            }
                break;
        }
    }

    return root;
}

static const struct mime_type_t zip_mimes_rw[] = {
    {   MIME_TYPE_FULL_NAME("application/zip", "zip", "Zip archive"),
        MIME_TYPE_COMPRESSION_VAL(0, 6, 9),
        MIME_TYPE_ENCRYPTION_NO,
        MIME_TYPE_COMPRESSION_MET_VAL(STRING_ARR("bzip2", "deflate", "store"), 1)
    }, {MIME_TYPE_FULL_NAME("application/x-java-archive", "jar", "JAR archive"),
        MIME_TYPE_COMPRESSION_VAL(0, 6, 9),
        MIME_TYPE_ENCRYPTION_NO,
        MIME_TYPE_ENCRYPTION_MET_NO
    },
    {MIME_TYPE_NULL}
};

static const struct cli_format_t cli_zip_proc = {
    .parent = {
        .name = "cli zip",
        .objectSize = sizeof(struct archive_t),
        .mime_types_rw = zip_mimes_rw,
        .flags = FORMAT_ENCRYPTION,

        .openArchive = format_default_openArchive,
        .listFiles = cli_listFiles,
        .extractFiles = cli_extractFiles,
        .deleteFiles = cli_deleteFiles,
        .testFiles = cli_testFiles,
        .addFiles = cli_addFiles,
        .closeArchive = format_default_closeArchive
    },
    .cmds = zip_cmds,
    .cmds_uses = zip_cmds_uses,

    .addSwitch = zip_addSwitch,
    .extractSwitch = zip_extractSwitch,
    .delSwitch = zip_delSwitch,
    .listSwitch = zip_listSwitch,
    .testSwitch = zip_testSwitch,

    .passwordSwitch = zip_passwordSwitch,
    .compressionLevelSwitch = zip_compressionLevelSwitch,
    .compressionMethodSwitch = zip_compressionMethodSwitch,

//    .errorWrongPassword = _7z_errorWrongPassword,
//    .errorCorruptedArchive = _7z_errorCorruptedArchive,
//    .errorFullDisk = _7z_errorFullDisk,
//    .fileExistsPatterns = _7z_fileExistsPatterns,
//    .fileExistsFileName = _7z_fileExistsFileName,
    .fileExistsInput = {"y", "n", "A", "N", NULL},

    .processList = cli_zip_processList
};

ADD_FORMAT(cli_zip_proc);
