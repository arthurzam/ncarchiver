#include "global.h"
#include "filetree.h"

#include <archive.h>
#include <archive_entry.h>

#include <stdio.h>

struct archive_libarchive_t {
	struct archive_t a;
    struct archive *reader, *writer;
};

static bool _libarchive_initReader(struct archive_libarchive_t *archive)
{
    if (archive->reader)
        archive_read_free(archive->reader);
    archive->reader = archive_read_new();

    if (!archive->reader)
    {
        fprintf(stderr, "The archive reader could not be initialized.\n");
        return false;
    }

    if (archive_read_support_filter_all(archive->reader) != ARCHIVE_OK)
        return false;

    if (archive_read_support_format_all(archive->reader) != ARCHIVE_OK)
        return false;

    if (archive_read_open_filename(archive->reader, archive->a.path, 10240) != ARCHIVE_OK)
    {
        fprintf(stderr, "Could not open the archive:%s\n", archive_error_string(archive->reader));
        return false;
    }

    return true;
}

static struct archive_t *libarchive_openArchive(const struct format_t *_archive, char *path)
{
    struct archive_libarchive_t *archive = (struct archive_libarchive_t *)malloc(sizeof(struct archive_libarchive_t));
    archive->a.path = path;
    archive->a.dir = NULL;
    archive->a.format = _archive;
    archive->a.flags = 0;

    archive->reader = archive_read_disk_new();
    archive_read_disk_set_standard_lookup(archive->reader);

    archive->writer = NULL;

    return &archive->a;
}

static struct dir_t *libarchive_listFiles(struct archive_t *_archive)
{
    struct archive_libarchive_t *archive = (struct archive_libarchive_t *)_archive;

    if (!_libarchive_initReader(archive))
        return NULL;

    struct dir_t *root = filetree_createRoot(), *temp;

    printf("compression filter: %s\n", archive_filter_name(archive->reader, 0));

    struct archive_entry *aentry;
    int result = ARCHIVE_RETRY;

    while ((result = archive_read_next_header(archive->reader, &aentry)) == ARCHIVE_OK)
    {
        const char *path = archive_entry_pathname_utf8(aentry);

        temp = filetree_addNode(root, (char*)path);
        if (S_ISDIR(archive_entry_mode(aentry)))
        {
            temp->realSize = temp->compressSize = 0;
            temp->flags |= NODE_ISDIR;
        }
        else
        {
            temp->realSize = archive_entry_size(aentry);
        }

        archive_read_data_skip(archive->reader);
    }

    archive_read_close(archive->reader);

    return root;
}

static bool libarchive_closeArchive(struct archive_t *_archive)
{
    struct archive_libarchive_t *archive = (struct archive_libarchive_t *)_archive;

    if (archive->reader)
        archive_read_free(archive->reader);
    if (archive->writer)
        archive_write_free(archive->writer);

    return format_default_closeArchive(_archive);
}

struct format_t libarchiveFormat = {
    .name = "libarchive",
    .openArchive = libarchive_openArchive,
    .listFiles = libarchive_listFiles,
    .closeArchive = libarchive_closeArchive
};

ADD_FORMAT(libarchiveFormat);
