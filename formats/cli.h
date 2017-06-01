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

    CLI_ALL = CLI_ADD | CLI_LIST | CLI_DELETE | CLI_RENAME | CLI_EXTRACT | CLI_COMPRESS,

    CLI_COUNT = 6
};

struct cli_format_t {
    struct format_t parent;

    char **cmds;
    const uint8_t *cmds_uses;

    const char **addSwitch;
    const char **extractSwitch;
    const char **delSwitch;
    const char **listSwitch;

    const char **passwordSwitch;

    struct dir_t *(*processList)(struct archive_t *archive, FILE *inF, FILE *outF) __attribute__ ((__nonnull__ (1,2,3)));
};

char *getCmdPath(const char *cmd) __attribute__((malloc));

struct dir_t *cli_listFiles(struct archive_t *_archive);

#define strstartswith(str, prefix) (strncmp(prefix, str, strlen(prefix)) == 0)

#endif // EXTERNAL_PROC_H
