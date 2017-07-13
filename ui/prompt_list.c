#include "global.h"
#include "global_ui.h"
#include "filetree.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>

struct prompt_list_t {
    const char *const *items;
    const char *title;
    unsigned selected_item, selected_btn, items_size, width;
};

static int prompt_list_key(int index, int key) {
    struct prompt_list_t *data = (struct prompt_list_t *)ui_data[index];
    switch(key) {
        case 9: // TAB
            data->selected_btn = 1 - data->selected_btn;
            break;
        case 10: // Enter
        case KEY_ENTER:
            return 1;
        case KEY_UP:
            if (data->selected_item > 0)
                --data->selected_item;
            break;
        case KEY_DOWN:
            if (data->selected_item < data->items_size - 1)
                ++data->selected_item;
            break;
        case KEY_HOME:
            data->selected_item = 0;
            break;
        case KEY_END:
            data->selected_item = data->items_size - 1;
            break;

    }
    return 0;
}

static void prompt_list_draw(int index) {
    struct prompt_list_t *data = (struct prompt_list_t *)ui_data[index];
    unsigned i;

    nccreate(data->items_size + 6, data->width, data->title);

    for (i = 0; i < data->items_size; ++i) {
        if (data->selected_item == i)
            attron(A_REVERSE);
        ncaddstr(i + 2, 2, data->items[i]);
        if (data->selected_item == i)
            attroff(A_REVERSE);
    }

    if (data->selected_btn == 0)
        attron(A_REVERSE);
    ncaddstr(data->items_size + 4, 2, "Select");
    if (data->selected_btn == 0)
        attroff(A_REVERSE);
    else
        attron(A_REVERSE);
    ncaddstr(data->items_size + 4, 11, "Cancel");
    if (data->selected_btn == 1)
        attroff(A_REVERSE);
}

unsigned prompt_list_init(const char *title, const char *const *items, unsigned defaultItem) {
    const char *const *ptr;
    unsigned width = 15, t = strlen(title) + 6;
    if (t > 15)
        width = t;
    for (ptr = items; *ptr; ptr++) {
        t = strlen(*ptr) + 4;
        if (t > width)
            width = t;
    }

    struct prompt_list_t data = {
        .title = title, .items = items, .selected_item = defaultItem, .selected_btn = 0,
        .items_size = ptr - items, .width = width + 4
    };
    ui_insert(prompt_list_draw, prompt_list_key, &data);
    while (input_handle(0) != 1);
    ui_remove();
    return data.selected_btn == 0 ? data.selected_item : defaultItem;
}
