#ifndef ACTIONS_H
#define ACTIONS_H

#include <global.h>

bool actions_openFiles(const char *const *files) __attribute__ ((__nonnull__));

/**
 * This function will try to delete all nodes.
 * If a delete on a node failed, it's place in nodes array will be nonnull.
 * All successful nodes that were deleted will be cleared from files tree and set to NULL in nodes array.
 *
 * return true only if all were successful
 */
bool actions_deleteFiles(struct dir_t **nodes, size_t nodes_size) __attribute__ ((__nonnull__));

#endif // ACTIONS_H
