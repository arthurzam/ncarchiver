#ifndef FILETREE_H
#define FILETREE_H

#include "global.h"

#include <stdlib.h>

struct dir_t *filetree_addNode(struct dir_t *root, char *path);
void filetree_free(struct dir_t *root);

enum SortFlags {
    SORT_COL_NAME = 0,
    SORT_COL_SIZE = 1,

    SORT_DIRS_FIRST = 0x10,
    SORT_REVERSE = 0x20
};
extern uint8_t sort_flags;

struct dir_t *filetree_sort(struct dir_t *list);

char *filetree_getpath(const struct dir_t *node);

struct dir_t *filetree_createRoot();

#endif // FILETREE_H
