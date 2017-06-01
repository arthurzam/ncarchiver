#include "global.h"
#include "global_ui.h"
#include "filetree.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>


// static int graph = 1, show_as = 0, info_show = 0, info_page = 0, info_start = 0, show_items = 0;
static char *message = NULL;
struct dir_t *curr, *selected;





static void browse_draw_item(struct dir_t *n, int row) {
    char type, *size, dt;
    int col = 0;

    if (n == selected)
        attron(A_REVERSE);
    if (n == curr->parent)
    {
        mvaddstr(row, 17, "/..");
        if (n == selected)
            attroff(A_REVERSE);
        return;
    }

    type = ((n->flags & NODE_ISDIR) ? (n->items > 0 ? 'D' : 'e') : ' ');
    size = ((n->flags & NODE_ISDIR) ? "" : formatsize(n->realSize));
    dt = ((n->flags & NODE_ISDIR) ? '/' : ' ');

    mvprintw(row, col, "%3s %c %9s ", ((n->flags & NODE_SELECTED) ? "(*)" : ""), type, size);
    col += 16;


    mvprintw(row, col, " %c%-*s", dt, wincols - 2 - col, cropstr(n->name, wincols - 2 - col));
    if (n == selected)
        attroff(A_REVERSE);
}


void browse_draw() {
  struct dir_t *t;
  char *tmp;
  int i = 2;

  erase();
  t = curr;

  /* top line - basic info */
  attron(A_REVERSE);
  mvhline(0, 0, ' ', wincols);
  mvhline(winrows-1, 0, ' ', wincols);
  mvprintw(0,0,"%s %s ~ Use the arrow keys to navigate, press ? for help", "PACKAGE_NAME", "PACKAGE_VERSION");
  attroff(A_REVERSE);

  /* second line - the path */
  mvhline(1, 0, '-', wincols);
    mvaddch(1, 3, ' ');
    tmp = filetree_getpath(curr);
    mvaddstr(1, 4, cropstr(tmp, wincols-8));
    mvaddch(1, 4+((int)strlen(tmp) > wincols-8 ? wincols-8 : (int)strlen(tmp)), ' ');

  /* nothing to display? stop here. */
  if(!t)
    return;

  /* get start position */
  t = curr->subs;

  if (curr->parent)
    browse_draw_item(curr->parent, i++);

  /* print the list to the screen */
  for(; t && i < winrows - 3; t = t->next, i++) {
    browse_draw_item(t, i);
  }

  /* move cursor to selected row for accessibility */
}


int browse_key(int ch) {

    switch (ch)
    {
        case KEY_UP:
            if (selected == curr->parent) break;
            if (selected->prev)
                selected = selected->prev;
            else if (curr->parent)
                selected = curr->parent;
            break;
        case KEY_DOWN:
            if (selected == curr->parent)
                selected = curr->subs;
            else if (selected->next)
                selected = selected->next;
            break;
        case '\n':
        case KEY_ENTER:
            if (selected->flags & NODE_ISDIR)
            {
                curr = selected;
                selected = filetree_sort(curr->subs);
            }
            break;
        case ' ':
            if (selected != curr->parent)
                selected->flags ^= NODE_SELECTED;
            break;
        case 'q':
            quit_init();
            break;
    }

  return 0;
}


void browse_init(struct dir_t *par) {
  pstate = ST_BROWSE;
  message = NULL;
  curr = par;
}

