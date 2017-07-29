#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "listview.h"
#include "functions.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>

#define WINDOW_WIDTH  60
#define WIN_MAX_HEIGHT (winrows / 2)

struct addfilesdialog_t {
    struct dir_t *destination;
    struct listview_data_t files;
};

enum SelRows {
    ROW_OK = -1,
    ROW_CANCEL = -2,

    ROW_FILES_LIST = 0
};

static int addfilesdialog_key(int index, int key) {
    struct addfilesdialog_t *data = (struct addfilesdialog_t *)ui_data[index];
    switch (key) {
        case KEY_HOME:
        case KEY_END:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_PPAGE:
        case KEY_NPAGE:
            listview_key(&data->files, key);
            break;
        case '-':
            if (data->files.selected_row >= ROW_FILES_LIST) {
                if (data->files.items_count == 1) {
                    arrfree(data->files.items);
                    data->files.items = NULL;
                    data->files.items_count = 0;
                } else if (data->files.items_count != 0) {
                    const unsigned entry = data->files.selected_row + data->files.first_row;
                    char **arr = data->files.items;
                    data->files.items = NULL;
                    arrremove(arr, entry);
                    listview_init(&data->files, arr, WIN_MAX_HEIGHT - 8, WINDOW_WIDTH, min(entry, data->files.items_count - 2));
                }
            }
            break;
        case '+':
            if (data->files.selected_row >= ROW_FILES_LIST) {
                char **arr = arrinsert(data->files.items, data->files.items_count, fselect_init(getHomeDir(), FSELECT_SELECT_FILES));
                data->files.items = NULL;
                listview_init(&data->files, arr, WIN_MAX_HEIGHT - 8, WINDOW_WIDTH, data->files.items_count);
            }
            break;
        case KEY_TAB:
            if (data->files.selected_row >= ROW_FILES_LIST)
                data->files.selected_row = ROW_OK;
            else if (data->files.selected_row == ROW_OK)
                data->files.selected_row = ROW_CANCEL;
            else
                data->files.selected_row = ROW_FILES_LIST;
            break;
        case KEY_ESC:
            return 2;
        case KEY_RETURN:
        case KEY_ENTER:
            if (data->files.selected_row == ROW_OK) {
                // TODO: add files
            } else if (data->files.selected_row == ROW_CANCEL)
                return 2;
    }
    return 0;
}

static void addfilesdialog_draw(int index) {
    const struct addfilesdialog_t *data = (const struct addfilesdialog_t *)ui_data[index];
    const unsigned height = max(data->files.height + 1, (unsigned)2);

    nccreate(height + 8, WINDOW_WIDTH, "Add files");

    ncaddstr(2, 2, "Destination:");
    attron(A_UNDERLINE);
    ncaddstr(2, 15, cropstr(filetree_getpath(data->destination), WINDOW_WIDTH - 19));
    attroff(A_UNDERLINE);

    ncaddstr(4, 3, "(+) to add (-) to remove");

    if (data->files.items) {
        subwinr += 6;
        listview_draw(&data->files);
        subwinr -= 6;
    } else {
        ncaddstr(6, 4, "[Empty list]");
    }

    if (data->files.selected_row == ROW_OK)
        attron(A_REVERSE);
    ncaddstr(6 + height, WINDOW_WIDTH / 4 - 1, "OK");
    if (data->files.selected_row == ROW_OK)
        attroff(A_REVERSE);
    else if (data->files.selected_row == ROW_CANCEL)
        attron(A_REVERSE);
    ncaddstr(6 + height, (3 * WINDOW_WIDTH / 4) - 3, "Cancel");
    if (data->files.selected_row == ROW_CANCEL)
        attroff(A_REVERSE);
}

void addfilesdialog_init(struct dir_t *destination, char **files) {
    struct addfilesdialog_t *data = TYPE_MALLOC(struct addfilesdialog_t);
    data->destination = destination;
    data->files.items = NULL;
    if (files)
        listview_init(&data->files, files, WIN_MAX_HEIGHT - 8, WINDOW_WIDTH, 0);
    else
        data->files.height = data->files.items_count = 0;

    ui_insert(addfilesdialog_draw, addfilesdialog_key, data);
}
