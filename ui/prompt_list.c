#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "listview.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>

#define WIN_MAX_HEIGHT (winrows / 2)

struct prompt_list_t {
    const char *title;
    struct listview_data_t list;
    unsigned selected_btn;
};

static int prompt_list_key(int index, int key) {
    struct prompt_list_t *data = (struct prompt_list_t *)ui_data[index];
    switch(key) {
        case KEY_TAB:
            data->selected_btn = 1 - data->selected_btn;
            break;
        case KEY_ENTER:
        case KEY_RETURN:
            return 1;
        case KEY_HOME:
        case KEY_END:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_PPAGE:
        case KEY_NPAGE:
            listview_key(&data->list, key);
            break;

    }
    return 0;
}

static void prompt_list_draw(int index) {
    struct prompt_list_t *data = (struct prompt_list_t *)ui_data[index];

    nccreate(data->list.height + 6, data->list.width, data->title);

    subwinr += 2;
    listview_draw(&data->list);
    subwinr -= 2;

    if (data->selected_btn == 0)
        attron(A_REVERSE);
    ncaddstr(data->list.height + 4, 2, "Select");
    if (data->selected_btn == 0)
        attroff(A_REVERSE);
    else
        attron(A_REVERSE);
    ncaddstr(data->list.height + 4, 11, "Cancel");
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

    struct prompt_list_t data = { .title = title, .selected_btn = 0 };
    data.list.items = NULL;
    listview_init(&data.list, (char **)items, WIN_MAX_HEIGHT - 6, width, defaultItem);
    ui_insert(prompt_list_draw, prompt_list_key, &data);
    while (input_handle(0) != 1);
    ui_remove();

    return data.selected_btn == 0 ? (unsigned)(data.list.first_row + data.list.selected_row) : defaultItem;
}
