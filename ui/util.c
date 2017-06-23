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

#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <stdarg.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

int winrows, wincols;
int subwinr, subwinc;
int si;
char thou_sep;
ui_draw_func_t ui_draw_funcs[UI_MAX_DEPTH] = {NULL};
ui_key_func_t  ui_key_funcs [UI_MAX_DEPTH] = {NULL};
void         * ui_data      [UI_MAX_DEPTH] = {NULL};


char *cropstr(const char *from, int s) {
  static char dat[4096];
  int i, j, o = strlen(from);
  if(o < s) {
    strcpy(dat, from);
    return dat;
  }
  j=s/2-3;
  for(i=0; i<j; i++)
    dat[i] = from[i];
  dat[i] = '.';
  dat[++i] = '.';
  dat[++i] = '.';
  j=o-s;
  while(++i<s)
    dat[i] = from[j+i];
  dat[s] = '\0';
  return dat;
}


char *formatsize(int64_t from) {
  static char dat[10]; /* "xxx.x MiB" */
  float r = from;
  char c = ' ';
  if (from == -1)
      return "-1";
  if (si) {
    if(r < 1000.0f)   { }
    else if(r < 1e6f) { c = 'K'; r/=1e3f; }
    else if(r < 1e9f) { c = 'M'; r/=1e6f; }
    else if(r < 1e12f){ c = 'G'; r/=1e9f; }
    else if(r < 1e15f){ c = 'T'; r/=1e12f; }
    else if(r < 1e18f){ c = 'P'; r/=1e15f; }
    else              { c = 'E'; r/=1e18f; }
    sprintf(dat, "%5.1f %cB", r, c);
  }
  else {
    if(r < 1000.0f)      { }
    else if(r < 1023e3f) { c = 'K'; r/=1024.0f; }
    else if(r < 1023e6f) { c = 'M'; r/=1048576.0f; }
    else if(r < 1023e9f) { c = 'G'; r/=1073741824.0f; }
    else if(r < 1023e12f){ c = 'T'; r/=1099511627776.0f; }
    else if(r < 1023e15f){ c = 'P'; r/=1125899906842624.0f; }
    else                 { c = 'E'; r/=1152921504606846976.0f; }
    sprintf(dat, "%5.1f %c%cB", r, c, c == ' ' ? ' ' : 'i');
  }
  return dat;
}


char *fullsize(int64_t from) {
  static char dat[26]; /* max: 9.223.372.036.854.775.807  (= 2^63-1) */
  char tmp[26];
  int64_t n = from;
  int i, j;

  /* the K&R method - more portable than sprintf with %lld */
  i = 0;
  do {
    tmp[i++] = n % 10 + '0';
  } while((n /= 10) > 0);
  tmp[i] = '\0';

  /* reverse and add thousand seperators */
  j = 0;
  while(i--) {
    dat[j++] = tmp[i];
    if(i != 0 && i%3 == 0)
      dat[j++] = thou_sep;
  }
  dat[j] = '\0';

  return dat;
}


void read_locale() {
  thou_sep = '.';
#ifdef HAVE_LOCALE_H
  setlocale(LC_ALL, "");
  char *locale_thou_sep = localeconv()->thousands_sep;
  if(locale_thou_sep && 1 == strlen(locale_thou_sep))
    thou_sep = locale_thou_sep[0];
#endif
}


int ncresize(int minrows, int mincols) {
  int ch;

  getmaxyx(stdscr, winrows, wincols);
  while((minrows && winrows < minrows) || (mincols && wincols < mincols)) {
    erase();
    mvaddstr(0, 0, "Warning: terminal too small,");
    mvaddstr(1, 1, "please either resize your terminal,");
    mvaddstr(2, 1, "press i to ignore, or press q to quit.");
    refresh();
    nodelay(stdscr, 0);
    ch = getch();
    getmaxyx(stdscr, winrows, wincols);
    if(ch == 'q') {
      erase();
      refresh();
      endwin();
      exit(0);
    }
    if(ch == 'i')
      return 1;
  }
  erase();
  return 0;
}


void nccreate(int height, int width, const char *title)
{
    int i;

    subwinr = winrows / 2 - height / 2;
    subwinc = wincols / 2 - width  / 2;

    /* clear window */
    for (i = 0; i < height; i++)
        mvhline(subwinr + i, subwinc, ' ', width);

    /* box() only works around curses windows, so create our own */
    move(subwinr, subwinc);
    addch(ACS_ULCORNER);
    for (i = 0; i < width - 2; i++)
        addch(ACS_HLINE);
    addch(ACS_URCORNER);

    move(subwinr + height - 1, subwinc);
    addch(ACS_LLCORNER);
    for (i = 0; i < width - 2; i++)
        addch(ACS_HLINE);
    addch(ACS_LRCORNER);

    mvvline(subwinr + 1, subwinc, ACS_VLINE, height - 2);
    mvvline(subwinr + 1, subwinc + width - 1, ACS_VLINE, height - 2);

    /* title */
    attron(A_BOLD);
    mvaddstr(subwinr, subwinc + 4, title);
    attroff(A_BOLD);
}


void ncprint(int r, int c, char *fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  move(subwinr+r, subwinc+c);
  vw_printw(stdscr, fmt, arg);
  va_end(arg);
}


void addparentstats(struct dir_t *d, int64_t size, int64_t asize, int items) {
  while(d) {
    d->realSize = adds64(d->realSize, size);
    d->compressSize = adds64(d->compressSize, asize);
    d->items += items;
    d = d->parent;
  }
}

int ui_insert(ui_draw_func_t draw, ui_key_func_t key, void *data) {
    int i;
    for (i = 0; ui_draw_funcs[i] != NULL; ++i);
    ui_draw_funcs[i] = draw;
    ui_data[i] = data;
    ui_key_funcs[i] = key;
    return i;
}

void *ui_remove() {
    int i;
    for (i = 0; ui_draw_funcs[i] != NULL; ++i);
    ui_draw_funcs[i - 1] = NULL;
    void *data = ui_data[i - 1];
    ui_data[i - 1] = NULL;
    ui_key_funcs[i - 1] = NULL;
    return data;
}
