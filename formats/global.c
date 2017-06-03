#include "global.h"
#include "filetree.h"

#include <stdlib.h>

struct archive_t *format_default_openArchive(const struct format_t *format, char *path)
{
    struct archive_t *archive = (struct archive_t *)malloc(sizeof(struct archive_t));
    archive->path = path;
    archive->dir = NULL;
    archive->format = format;
    archive->flags = 0;

    return archive;
}

bool format_default_closeArchive(struct archive_t *archive)
{
    filetree_free(archive->dir);
    free(archive->comment);
    free(archive);
    return true;
}
