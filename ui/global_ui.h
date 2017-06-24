#ifndef GLOBAL_UI_H
#define GLOBAL_UI_H

#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

# include <inttypes.h>
# include <stdint.h>

#define PACKAGE_NAME "ncarchiver"
#define PACKAGE_VERSION "0.0"

extern struct archive_t *arc;

/* read-only flag */
// extern int read_only;
/* minimum screen update interval when calculating, in ms */
extern long update_delay;

/* handle input from keyboard and update display */
int input_handle(int);

/* import all other global functions and variables */
#include "util.h"
#include "prompt.h"

void browse_init(struct dir_t *base);
void nodeinfo_init(struct dir_t *node);
void compressdialog_init();

#endif // GLOBAL_UI_H
