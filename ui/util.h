/* ncdu - NCurses Disk Usage

  Copyright (c) 2007-2016 Yoran Heling

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef _util_h
#define _util_h

#include "global.h"
#include <ncurses.h>

/* updated when window is resized */
extern int winrows, wincols;

/* used by the nc* functions and macros */
extern int subwinr, subwinc;

/* used by formatsize to choose between base 2 or 10 prefixes */
extern int si;


/* Instead of using several ncurses windows, we only draw to stdscr.
 * the functions nccreate, ncprint and the macros ncaddstr and ncaddch
 * mimic the behaviour of ncurses windows.
 * This works better than using ncurses windows when all windows are
 * created in the correct order: it paints directly on stdscr, so
 * wrefresh, wnoutrefresh and other window-specific functions are not
 * necessary.
 * Also, this method doesn't require any window objects, as you can
 * only create one window at a time.
*/

/* updates winrows, wincols, and displays a warning when the terminal
 * is smaller than the specified minimum size. */
int ncresize(int, int);

/* creates a new centered window with border */
void nccreate(int, int, const char *);

/* printf something somewhere in the last created window */
void ncprint(int, int, char *, ...);

/* same as the w* functions of ncurses */
#define ncaddstr(r, c, s) mvaddstr(subwinr+(r), subwinc+(c), s)
#define  ncaddch(r, c, s)  mvaddch(subwinr+(r), subwinc+(c), s)
#define   ncmove(r, c)        move(subwinr+(r), subwinc+(c))

/* crops a string into the specified length */
char *cropstr(const char *, int);

/* formats size in the form of xxx.xXB */
char *formatsize(int64_t );

/* int2string with thousand separators */
char *fullsize(int64_t);

/* read locale information from the environment */
void read_locale();

/* Add two signed 64-bit integers. Returns INT64_MAX if the result would
 * overflow, or 0 if it would be negative. At least one of the integers must be
 * positive.
 * I use uint64_t's to detect the overflow, as (a + b < 0) relies on undefined
 * behaviour, and (INT64_MAX - b >= a) didn't work for some reason. */
#define adds64(a, b) ((a) > 0 && (b) > 0\
    ? ((uint64_t)(a) + (uint64_t)(b) > (uint64_t)INT64_MAX ? INT64_MAX : (a)+(b))\
    : (a)+(b) < 0 ? 0 : (a)+(b))

/* Adds a value to the size, asize and items fields of *d and its parents */
void addparentstats(struct dir_t *, int64_t, int64_t, int);

typedef void(*ui_draw_func_t)(int index);
/*
 * returns:
 *  2 - close window
 *  1 - exit key loop
 *  0 - accepted key press
 */
typedef int (*ui_key_func_t )(int index, int key);

#define UI_MAX_DEPTH 16
extern ui_draw_func_t ui_draw_funcs[UI_MAX_DEPTH];
extern ui_key_func_t  ui_key_funcs [UI_MAX_DEPTH];
extern void         * ui_data      [UI_MAX_DEPTH];

int ui_insert(ui_draw_func_t draw, ui_key_func_t key, void *data);
void *ui_remove();

#endif

