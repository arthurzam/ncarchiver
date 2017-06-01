#include "global_ui.h"

#include <ncurses.h>

int quit_key(int ch) {
    switch(ch) {
        case 'y':
        case 'Y':
            return 1;
        default:
            pstate = ST_BROWSE;
    }
    return 0;
}

void quit_draw() {
    // browse_draw();

    nccreate(4,30, "ncdu confirm quit");
    ncaddstr(2,2, "Really quit? (y/N)");
}

void quit_init() {
    pstate = ST_QUIT;
}
