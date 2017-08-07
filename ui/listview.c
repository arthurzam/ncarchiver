#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "actions.h"
#include "functions.h"
#include "listview.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>

void listview_key(struct listview_data_t *data, int key) {
    switch (key) {
        case KEY_HOME:
            data->first_row = data->selected_row = 0;
            break;
        case KEY_END:
            if (data->height == data->items_count) {
                data->first_row = 0;
                data->selected_row = data->items_count - 1;
            } else {
                data->selected_row = (int)data->height - 1;
                data->first_row = data->items_count - (int)data->height;
            }
            break;
        case KEY_UP:
            if (data->selected_row < 0)
                break;
            if (data->selected_row != 0)
                --data->selected_row;
            else if (data->first_row != 0)
                --data->first_row;
            break;
        case KEY_DOWN:
            if (data->selected_row < 0)
                break;
            if (data->selected_row + data->first_row == (int)data->items_count - 1)
                break;
            if (data->selected_row < (int)data->height - 1)
                ++data->selected_row;
            else
                ++data->first_row;
            break;
        case KEY_PPAGE:
            if (data->first_row >= (int)data->height)
                data->first_row -= data->height;
            else
                data->first_row = data->selected_row = 0;
            break;
        case KEY_NPAGE:
            if (data->height == data->items_count) {
                data->first_row = 0;
                data->selected_row = data->items_count - 1;
            } else {
                data->first_row += data->height;
                if (data->first_row >= (int)(data->items_count - data->height)) {
                    data->selected_row = (int)data->height - 1;
                    data->first_row = data->items_count - (int)data->height;
                }
            }
            break;
    }
}

void listview_draw(const struct listview_data_t *data) {
    for (int i = 0; i < (int)data->height; ++i) {
        if (i == data->selected_row)
            attron(A_REVERSE);
        ncaddstr(i, 2, cropstr(data->items[data->first_row + i], data->width - 5));
        if (i == data->selected_row)
            attroff(A_REVERSE);
    }
}

void listview_init(struct listview_data_t *data, char **items, unsigned height, unsigned width, unsigned selected) {
    NC_ASSERT(data->items != items, "arrays shouldn't be set already");
    if (data->items)
        arrfree(data->items);
    data->items = items;
    data->items_count = arrlen((const char *const *)items);
    data->height = min(height, data->items_count);
    data->width = width;
    if (selected < data->height) { // only one page
        data->selected_row = selected;
        data->first_row = 0;
    } else if (selected + data->height >= data->items_count) { // last page
        data->first_row = (int)data->items_count - (int)data->height;
        data->selected_row = (int)selected - data->first_row;
    } else { // middle page
        data->selected_row = 0;
        data->first_row = selected;
    }
}
