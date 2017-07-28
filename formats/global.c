#include "global.h"
#include "filetree.h"

#include <string.h>
#include <ncurses.h>

bool format_default_openArchive(struct archive_t *archive, char *path) {
    archive->path = strdup(path);
    archive->dir = NULL;
    archive->password = NULL;
    archive->comment = NULL;

    return true;
}

bool format_default_closeArchive(struct archive_t *archive) {
    filetree_free(archive->dir);
    free(archive->comment);
    free(archive->password);
    free(archive->path);
    free(archive);
    return true;
}

#ifndef NDEBUG
void _nc_abort(const char *file, int line, const char *msg) {
    fprintf(loggerFile, "%s:%d Assert Error: %s\n", file, line, msg);
    fclose(loggerFile);
    erase();
    refresh();
    endwin();
    fprintf(stderr, "%s:%d Assert Error: %s\n", file, line, msg);
    exit(1);
}
#endif
