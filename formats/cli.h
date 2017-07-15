#ifndef EXTERNAL_PROC_H
#define EXTERNAL_PROC_H

#include <stdint.h>
#include <stdio.h>

#include "global.h"

enum CLI_USE {
    CLI_ADD      = 0x1,
    CLI_LIST     = 0x2,
    CLI_DELETE   = 0x4,
    CLI_RENAME   = 0x8,
    CLI_EXTRACT  = 0x10,
    CLI_COMPRESS = 0x20,
    CLI_TEST     = 0x40,

    CLI_ALL = CLI_ADD | CLI_LIST | CLI_DELETE | CLI_RENAME | CLI_EXTRACT | CLI_COMPRESS | CLI_TEST,

    CLI_COUNT = 6
};

struct pMimeStr {
    const char *mime;
    const char *str;
};
struct cli_format_t {
    struct format_t parent;

    const char *const *cmds;
    const uint8_t *cmds_uses;

    const char **addSwitch;
    const char **extractSwitch;
    const char **delSwitch;
    const char **listSwitch;
    const char **testSwitch;

    const char **passwordSwitch;
    const char **passwordHeadersSwitch;

    const struct pMimeStr* compressionLevelSwitch;
    const struct pMimeStr* compressionMethodSwitch;
    const struct pMimeStr* encryptionMethodSwitch;

    const char *const *errorWrongPassword;
    const char *const *errorCorruptedArchive;
    const char *const *errorFullDisk;
    const char *const *fileExistsPatterns;
    const char *const *fileExistsFileName;
    const char *fileExistsInput[5];

    struct dir_t *(*processList)(struct archive_t *archive, FILE *inF, FILE *outF) __attribute__ ((__nonnull__ (1,2,3)));
    int (*processLine)(struct archive_t *archive, const char *line, FILE *inF, int *flags) __attribute__ ((__nonnull__ (1,2,3,4)));
};

char *getCmdPath(const char *cmd) __attribute__((malloc)) __attribute__ ((__nonnull__ (1)));
bool start_subprocess(int *pid, int *infd, int *outfd, const char *cmd, char **argv, const char *cwd) __attribute__ ((__nonnull__ (1, 4, 5)));

int cli_processLineErrors(struct archive_t *archive, const char *line, FILE *inF, int *flags);
struct dir_t *cli_listFiles(struct archive_t *archive);
bool cli_extractFiles(struct archive_t *archive, const char *const *files, const char *destinationFolder);
int cli_deleteFiles(struct archive_t *archive, char **files);
bool cli_testFiles(struct archive_t *archive);
bool cli_addFiles(struct archive_t *archive, const char *const *files, const struct compression_options_t *options);

#define strstartswith(str, prefix) (strncmp(prefix, str, strlen(prefix)) == 0)

#endif // EXTERNAL_PROC_H
