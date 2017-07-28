#include "global.h"
#include "global_ui.h"
#include "filetree.h"
#include "functions.h"
#include "listview.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>

#define WIN_MAX_HEIGHT (winrows / 2)
#define WIN_WIDTH      60

struct fselect_t {
    char *path;
    struct listview_data_t list;
    int flags;
};

static int _sort_strcmp(const void *_a, const void *_b) {
    const char *a = *(const char **)_a;
    const char *b = *(const char **)_b;
    if (a[0] == '/' && b[0] != '/')
        return -1;
    if (b[0] == '/' && a[0] != '/')
        return 1;
    return strcmp(a, b);
}

static char **getDirContent(const char *path, int flags) {
    unsigned arr_len = 0, len;
    char **arr = NULL;
    const bool isRoot = path[0] == '/' && path[1] == '\0';
    if (!isRoot) {
        arr = (char **)malloc(sizeof(char *) * 2);
        arr[0] = strdup("/..");
        arr[1] = NULL;
        arr_len++;
    }

    DIR* dir = opendir(path);
    if (!dir)
        return arr;

    struct dirent *ent;

    while ((ent = readdir(dir))) {
        if(ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;
        if ((flags & 0x1) && ent->d_type != DT_DIR) // dirs only
            continue;
        arr = (char **)realloc(arr, sizeof(char *) * (arr_len + 2));
        len = strlen(ent->d_name);
        arr[arr_len] = (char *)malloc(len + (ent->d_type == DT_DIR ? 2 : 1));
        if (ent->d_type == DT_DIR)
            arr[arr_len][0] = '/';
        memcpy(arr[arr_len] + !!(ent->d_type == DT_DIR), ent->d_name, len + 1);
        arr_len++;
    }
    arr[arr_len] = NULL;
    qsort(arr + !isRoot, arr_len - !isRoot, sizeof(char *), _sort_strcmp);
    closedir(dir);
    return arr;
}

static int fselect_key(int index, int key) {
    struct fselect_t *data = (struct fselect_t *)ui_data[index];
    char *ptr;
    switch (key) {
        case KEY_HOME:
        case KEY_END:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_PPAGE:
        case KEY_NPAGE:
            if (data->list.selected_row >= 0)
                listview_key(&data->list, key);
            break;
        case '\t':
            if (data->list.selected_row >= 0)
                data->list.selected_row = -1;
            else if (data->list.selected_row == -1)
                data->list.selected_row = -2;
            else
                data->list.selected_row = 0;
            break;
        case KEY_RETURN:
        case KEY_ENTER:
            if (data->list.selected_row < 0)
                return 1;
            ptr = data->list.items[data->list.first_row + data->list.selected_row];
            if (ptr[0] == '/') {
                if (0 == strcmp(ptr, "/..")) {
                    if (data->path != (ptr = strrchr(data->path, '/')))
                        *ptr = '\0';
                    else
                        data->path[1] = '\0';
                } else {
                    data->path = (char *)realloc(data->path, strlen(data->path) + strlen(ptr) + 1);
                    if (data->path[1] != '\0')
                        strcat(data->path, ptr);
                    else
                        strcpy(data->path, ptr);
                }
                listview_init(&data->list, getDirContent(data->path, data->flags), WIN_MAX_HEIGHT - 7, WIN_WIDTH, 0);
            }
            break;
    }
    return 0;
}

static void fselect_draw(int index) {
    const struct fselect_t *data = (const struct fselect_t *)ui_data[index];

    nccreate(data->list.height + 7, WIN_WIDTH, "Select File");

    attron(A_REVERSE);
    ncprint(2, 2, "%-*s", WIN_WIDTH - 4, data->path);
    attroff(A_REVERSE);

    subwinr += 4;
    listview_draw(&data->list);
    subwinr -= 4;

    if (data->list.selected_row == -1)
        attron(A_REVERSE);
    ncaddstr(5 + data->list.height, WIN_WIDTH / 4 - 3, "Select");
    if (data->list.selected_row == -1)
        attroff(A_REVERSE);
    else if (data->list.selected_row == -2)
        attron(A_REVERSE);
    ncaddstr(5 + data->list.height, (3 * WIN_WIDTH / 4) - 3, "Cancel");
    if (data->list.selected_row == -2)
        attroff(A_REVERSE);
}

char *fselect_init(const char *path) {
    struct fselect_t data;
    data.path = strdup(path);
    data.flags = 0;
    data.list.items = NULL;
    listview_init(&data.list, getDirContent(path, data.flags), WIN_MAX_HEIGHT - 7, WIN_WIDTH, 0);

    ui_insert(fselect_draw, fselect_key, &data);
    while (input_handle(0) != 1);
    ui_remove();

    arrfree(data.list.items);

    return data.path;
}
