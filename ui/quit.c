#include "global_ui.h"

#include <ncurses.h>

int quit_key(int index, int ch) {
    (void)index;
    switch(ch) {
        case 'y':
        case 'Y':
            return 1;
        default:
            return 2;
    }
    return 0;
}

void quit_draw(int index) {
    (void)index;
    // browse_draw();

    nccreate(4,30, "ncdu confirm quit");
    ncaddstr(2,2, "Really quit? (y/N)");
}

void quit_init() {
    ui_insert(quit_draw, quit_key, NULL);
}
