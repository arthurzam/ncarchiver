#ifndef GLOBAL_UI_H
#define GLOBAL_UI_H

//#include "config.h"
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
extern int read_only;
/* minimum screen update interval when calculating, in ms */
extern long update_delay;
/* filter directories with CACHEDIR.TAG */
extern int cachedir_tags;
/* flag if we should ask for confirmation when quitting */
extern int confirm_quit;

/* handle input from keyboard and update display */
int input_handle(int);

/* import all other global functions and variables */
#include "util.h"
#include "quit.h"
#include "inputtext.h"

#endif // GLOBAL_UI_H
