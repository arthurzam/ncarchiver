#include "global.h"
#include "global_ui.h"
#include "filetree.h"

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>

static int nodeinfo_key(int index, int ch) {
    // struct dir_t *node = (struct dir_t *)ui_data[index];
    switch (ch) {
        case 'd':
        case 'D':
            if (prompy_yesno("Delete file", "Are you sure you want to delete this file?", 46))
            {
                // TODO: delete file
            }
            break;
        case 'o':
        case 'O':
            // TODO: open file
            break;
        case 'Q':
        case 'q':
            ui_data[index] = NULL;
            return 2;
    }
    return 0;
}

static void nodeinfo_draw(int index) {

    struct dir_t *node = (struct dir_t *)ui_data[index];
    const char *path = filetree_getpath(node);
    unsigned moreCount = 0, i;
    unsigned row = 2;
    unsigned width = 26; // length of buttons
    if (width < (i = strlen(path) + 6))
        width = i;
    if (node->moreInfo)
        for (; node->moreInfo[moreCount].key != NULL; ++moreCount)
            if (width < (i = strlen(node->moreInfo[moreCount].key) + strlen(node->moreInfo[moreCount].value) + 2))
                width = i;

    nccreate(9 + moreCount, width + 4, "Info");
    ncprint(row++, 2, "Name: %s", node->name);
    ncprint(row++, 2, "Path: %s", filetree_getpath(node));
    ncprint(row++, 2, "File Size: %d", node->realSize);
    ncprint(row++, 2, "Compressed Size: %d", node->realSize);
    if (node->moreInfo)
        for (i = 0; node->moreInfo[i].key != NULL; ++i)
            ncprint(row++, 2, "%s: %s", node->moreInfo[i].key, node->moreInfo[i].value);
    row++;

    ncaddstr(row, 2, "(Q)uit   (D)elete   (O)pen");
}

void nodeinfo_init(struct dir_t *node) {
    ui_insert(nodeinfo_draw, nodeinfo_key, node);
}
