#include "global.h"
#include "filetree.h"

#include <string.h>

bool format_default_openArchive(struct archive_t *archive, char *path)
{
    archive->path = strdup(path);
    archive->dir = NULL;
    archive->flags = 0;
    archive->password = NULL;
    archive->comment = NULL;

    return true;
}

bool format_default_closeArchive(struct archive_t *archive)
{
    filetree_free(archive->dir);
    free(archive->comment);
    free(archive->password);
    free(archive->path);
    free(archive);
    return true;
}
