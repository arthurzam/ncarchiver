#include "global_ui.h"

#include <ncurses.h>

int inputtext_key(int ch) {
    switch(ch) {
//        case KEY_ENTER:
//            // TODO: accept
//            break;
//        default:

    }
    return 0;
}

void inputtext_draw() {
    // browse_draw();

    nccreate(4,30, "ncdu confirm quit");
    ncaddstr(2,2, "Really quit? (y/N)");
}

void inputtext_init() {
    pstate = ST_QUIT;
}
