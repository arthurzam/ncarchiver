#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "listview.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>

#define WINDOW_WIDTH  60
#define WINDOW_HEIGHT 11

enum Flags {
    FLAG_JUNK_PATH = 0x1,
    FLAG_ALL_FILES = 0x2
};

struct extractdialog_t {
    char *destination;
    struct dir_t **files;
    unsigned files_size;
    unsigned selectedObj;
    int flags;
};

enum SelRows {
    ROW_OK = 0,
    ROW_CANCEL,
    ROW_DESTINATION,

    ROW_FILES_ALL,
    ROW_FILES_SELECTED,

    ROW_RECREATE_PATH
};

static int extractdialog_key(int index, int key) {
    struct extractdialog_t *data = (struct extractdialog_t *)ui_data[index];
    switch (key) {
        case KEY_HOME:
            data->selectedObj = ROW_DESTINATION;
            break;
        case KEY_END:
            data->selectedObj = ROW_RECREATE_PATH;
            break;
        case KEY_UP:
            if (data->selectedObj == ROW_OK)
                data->selectedObj = ROW_RECREATE_PATH;
            else
                --data->selectedObj;
            break;
        case KEY_TAB:
        case KEY_DOWN:
            if (data->selectedObj == ROW_RECREATE_PATH)
                data->selectedObj = ROW_OK;
            else
                ++data->selectedObj;
            break;
        case KEY_ESC:
        case 'q':
        case 'Q':
            if (data->selectedObj != ROW_DESTINATION)
                goto _close;
            break;
        case KEY_ENTER:
        case KEY_RETURN:
            if (data->selectedObj == ROW_CANCEL)
                goto _close;
            else if (data->selectedObj == ROW_OK) {
                // TODO: extract files
                goto _close;
            } else if (data->selectedObj == ROW_DESTINATION) {
                char *res = fselect_init(data->destination, FSELECT_DIRS_ONLY);
                free(data->destination);
                data->destination = res;
                break;
            }
            // fallthrough
        case KEY_SPACE:
            if (data->selectedObj == ROW_RECREATE_PATH)
                data->flags ^= FLAG_JUNK_PATH;
            else if (data->selectedObj == ROW_FILES_ALL || data->selectedObj == ROW_FILES_SELECTED)
                data->flags ^= FLAG_ALL_FILES;
            break;
    }
    return 0;
_close:
    free(data->destination);
    data->destination = NULL;
    return 2;
}

static void extractdialog_draw(int index) {
    struct extractdialog_t *data = (struct extractdialog_t *)ui_data[index];

    nccreate(WINDOW_HEIGHT, WINDOW_WIDTH, "Extract Files");

    draw_label(2, 2, "Destination:", data->selectedObj == ROW_DESTINATION);
    attron(A_UNDERLINE);
    ncaddstr(2, 15, cropstr(data->destination, WINDOW_WIDTH - 19));
    attroff(A_UNDERLINE);

    draw_label(4, 2, (data->flags & FLAG_ALL_FILES) ? "[*]" : "[ ]", data->selectedObj == ROW_FILES_ALL);
    ncaddstr  (4, 6, "All files");
    draw_label(5, 2, (data->flags & FLAG_ALL_FILES) ? "[ ]" : "[*]", data->selectedObj == ROW_FILES_SELECTED);
    ncprint   (5, 6, "Selected files (%d)", data->files_size);

    draw_label(7, 2, (data->flags & FLAG_JUNK_PATH) ? "[ ]" : "[X]", data->selectedObj == ROW_RECREATE_PATH);
    ncaddstr  (7, 6, "Recreate paths");

    draw_label(9, WINDOW_WIDTH / 4 - 1,       "OK"    , data->selectedObj == ROW_OK);
    draw_label(9, (3 * WINDOW_WIDTH / 4) - 3, "Cancel", data->selectedObj == ROW_CANCEL);
}

void extractdialog_init(struct dir_t **files, unsigned files_size) {
    struct extractdialog_t *data = TYPE_MALLOC(struct extractdialog_t);
    *data = (struct extractdialog_t) {
        .destination = strdup(getHomeDir()), files = files, .files_size = files_size,
        .selectedObj = ROW_DESTINATION, .flags = (files_size == 0 ? FLAG_ALL_FILES : 0)
    };
    ui_insert(extractdialog_draw, extractdialog_key, data);
}
