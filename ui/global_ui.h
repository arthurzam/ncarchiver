#ifndef GLOBAL_UI_H
#define GLOBAL_UI_H

#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <inttypes.h>
#include <stdint.h>

#include "keys.h"

#define PACKAGE_NAME "ncarchiver"
#define PACKAGE_VERSION "0.0"

extern struct archive_t *arc;

/* handle input from keyboard and update display */
int input_handle(int);

/* import all other global functions and variables */
#include "util.h"
#include "prompt.h"

void browse_init(struct dir_t *base);
void nodeinfo_init(struct dir_t *node);
struct compression_options_t *newfiledialog_init();
void addfilesdialog_init(struct dir_t *destination, char **files);
void extractdialog_init(struct dir_t **files, unsigned files_size);
unsigned prompt_list_init(const char *title, const char *const *items, unsigned defaultItem);

enum FselectFlags {
    FSELECT_DIRS_ONLY = 0x1,
    FSELECT_SELECT_FILES = 0x2
};

char *fselect_init(const char *path, unsigned flags);

/* widgets */
void draw_label(int row, int col, const char *text, bool flag);

#endif // GLOBAL_UI_H
