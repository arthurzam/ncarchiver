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

/* Program states */
#define ST_CALC   0
#define ST_BROWSE 1
#define ST_DEL    2
#define ST_HELP   3
#define ST_SHELL  4
#define ST_QUIT   5


/* program state */
extern int pstate;
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
#include "browser.h"
//#include "delete.h"
//#include "dir.h"
//#include "dirlist.h"
//#include "exclude.h"
//#include "help.h"
//#include "path.h"
#include "util.h"
//#include "shell.h"
#include "quit.h"

#endif // GLOBAL_UI_H
