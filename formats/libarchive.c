#include "global.h"
#include "filetree.h"

#include <archive.h>
#include <archive_entry.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>

struct archive_libarchive_t {
	struct archive_t a;
    struct archive *reader, *writer;
};

static bool _libarchive_initReader(struct archive_libarchive_t *archive) {
    if (archive->reader)
        archive_read_free(archive->reader);
    archive->reader = archive_read_new();

    if (!archive->reader) {
        LOG_e("libarchive", "The archive reader could not be initialized");
        return false;
    }

    if (archive_read_support_filter_all(archive->reader) != ARCHIVE_OK)
        return false;

    if (archive_read_support_format_all(archive->reader) != ARCHIVE_OK)
        return false;

    if (archive_read_open_filename(archive->reader, archive->a.path, 10240) != ARCHIVE_OK) {
        LOG_E("libarchive", "Could not open the archive: %s", archive_error_string(archive->reader));
        return false;
    }

    return true;
}

static bool _libarchive_initWriter(struct archive_libarchive_t *archive, bool newFile, struct compression_options_t *options) {
    if (archive->writer)
        archive_write_free(archive->writer);
    archive->writer = archive_write_new();

    if (!archive->writer) {
        LOG_e("libarchive", "The archive writer could not be initialized");
        return false;
    }

    archive_write_set_format_pax_restricted(archive->writer);

    int ret;
    bool requiresExecutable;

    if (newFile) {
        typedef int(*filterFunc)(struct archive *);
        static const struct data_t { const char *extension; filterFunc filter; } extenFilter[] = {
            {"GZ",   archive_write_add_filter_gzip},
            {"BZ2",  archive_write_add_filter_bzip2},
            {"XZ",   archive_write_add_filter_xz},
            {"LZMA", archive_write_add_filter_lzma},
            {".Z",   archive_write_add_filter_compress},
            {".LZ",  archive_write_add_filter_lzip},
            {"LZO",  archive_write_add_filter_lzop},
            {"LRZ",  archive_write_add_filter_lrzip},
            {"LZ4",  archive_write_add_filter_lz4},
            {"TAR",  archive_write_add_filter_none},
        };
        const struct data_t *iter, *const end = extenFilter + (sizeof(extenFilter) / sizeof(extenFilter[0]));
        int len = strlen(archive->a.path);
        for (iter = extenFilter; extenFilter != end; ++iter) {
            if (0 == strcasecmp(archive->a.path + len - strlen(iter->extension), iter->extension)) {
                LOG_I("libarchive", "Detected %s compression for new file", iter->extension);
                ret = iter->filter(archive->writer);
                requiresExecutable = (iter->filter == archive_write_add_filter_lrzip);
                break;
            }
        }
        if (extenFilter == end) {
            LOG_i("libarchive", "Falling back to gzip");
            ret = archive_write_add_filter_gzip(archive->writer);
        }
    } else {
        ret = archive_filter_code(archive->reader, 0);
        requiresExecutable = (ret == ARCHIVE_FILTER_LRZIP);
        ret = archive_write_add_filter(archive->writer, ret);
    }

    if (requiresExecutable ? ret != ARCHIVE_WARN : ret != ARCHIVE_OK) {
        LOG_E("libarchive", "Failed to set compression method: %s", archive_error_string(archive->writer));
        return false;
    }

    if (newFile && options) {
        if (options->compressionLevel != -1) {
            char lev[] = "0";
            lev[0] += options->compressionLevel;
            ret = archive_write_set_filter_option(archive->writer, NULL, "compression-level", lev);
            if (ret != ARCHIVE_OK) {
                LOG_E("libarchive", "Failed to set compression level: %s", archive_error_string(archive->writer));
                return false;
            }
        }
    }

    if (archive_write_open_filename(archive->writer, archive->a.path) != ARCHIVE_OK) {
        LOG_E("libarchive", "Could not open the archive for writing entries: %s", archive_error_string(archive->writer));
        return false;
    }

    return true;
}

static bool _libarchive_writeAEntry(struct archive_libarchive_t *archive, struct archive_entry *entry) {
    char buff[10240];
    la_ssize_t readBytes;
    switch (archive_write_header(archive->writer, entry)) {
        case ARCHIVE_OK:
            while ((readBytes = archive_read_data(archive->reader, buff, sizeof(buff))) > 0) {
                archive_write_data(archive->writer, buff, readBytes);
                if (archive_errno(archive->writer) != ARCHIVE_OK) {
                    // TODO: print error https://github.com/KDE/ark/blob/master/plugins/libarchive/libarchiveplugin.cpp#L536
                }
            }
            break;
        case ARCHIVE_FAILED:
        case ARCHIVE_FATAL:
            return false;
    }
    return true;
}

static bool _libarchive_processOldEntries(struct archive_libarchive_t *archive, unsigned totalCount, char **files) {
    (void)totalCount;
    struct archive_entry *entry;
    unsigned iteratedEntries = 0;
    char **ptr;

    while (archive_read_next_header(archive->reader, &entry) == ARCHIVE_OK) {
        const char *entryPath = archive_entry_pathname(entry);
        if (archive->a.op & (OP_MOVE | OP_COPY)) {
            // TODO: https://github.com/KDE/ark/blob/master/plugins/libarchive/readwritelibarchiveplugin.cpp#L413
        } else {
            for (ptr = files; *ptr; ptr++)
                if (0 == strcmp(entryPath, *ptr)) {
                    archive_read_data_skip(archive->reader);
                    switch (archive->a.op) {
                        case OP_DELETE:
                            // TODO: show progress
                            break;
                        case OP_ADD:
                            LOG_D("libarchive", "File exists, skipping: %s", entryPath);
                            break;
                    }
                    break;
                }
            if (*ptr) {
                *ptr = NULL;
                continue;
            }
        }

        if (_libarchive_writeAEntry(archive, entry)) {
            if (archive->a.op == OP_DELETE)
                iteratedEntries++;
        }
        // TODO: show progress
    }
    return true;
}

static bool libarchive_openArchive(struct archive_t *_archive, char *path) {
    struct archive_libarchive_t *archive = (struct archive_libarchive_t *)_archive;

    archive->reader = archive_read_disk_new();
    archive_read_disk_set_standard_lookup(archive->reader);

    archive->writer = NULL;

    return format_default_openArchive(&archive->a, path);
}

static struct dir_t *libarchive_listFiles(struct archive_t *_archive)
{
    struct archive_libarchive_t *archive = (struct archive_libarchive_t *)_archive;

    if (!_libarchive_initReader(archive))
        return NULL;

    struct dir_t *root = filetree_createRoot(), *temp;

    LOG_I("libarchive", "compression filter: %s", archive_filter_name(archive->reader, 0));

    struct archive_entry *aentry;
    int result = ARCHIVE_RETRY;

    while ((result = archive_read_next_header(archive->reader, &aentry)) == ARCHIVE_OK) {
        const char *path = archive_entry_pathname_utf8(aentry);

        temp = filetree_addNode(root, (char*)path);
        if (S_ISDIR(archive_entry_mode(aentry))) {
            temp->realSize = temp->compressSize = 0;
            temp->flags |= NODE_ISDIR;
        } else {
            temp->realSize = archive_entry_size(aentry);
        }

        archive_read_data_skip(archive->reader);
    }

    archive_read_close(archive->reader);

    return root;
}

int libarchive_deleteFiles(struct archive_t *_archive, char **files) {
    struct archive_libarchive_t *archive = (struct archive_libarchive_t *)_archive;

    if (!_libarchive_initReader(archive))
        return false;

    if (!_libarchive_initWriter(archive, false, NULL))
        return false;

    archive->a.op = OP_DELETE;
    return _libarchive_processOldEntries(archive, 0, files);
    // TODO: continue work
    return false;
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

static const char *libarchive_mimes_ro[] = {
    "application/x-deb",
    "application/x-cd-image",
    "application/x-bcpio",
    "application/x-cpio",
    "application/x-cpio-compressed",
    "application/x-sv4cpio",
    "application/x-sv4crc",
    "application/x-rpm",
    "application/x-source-rpm",
    "application/vnd.debian.binary-package",
    "application/vnd.ms-cab-compressed",
    "application/x-xar",
    "application/x-iso9660-appimage",
    "application/x-archive",
    NULL,
};

static const struct mime_type_t libarchive_mimes_rw[] = {
    {MIME_TYPE_FULL_NAME_ONLY("application/x-tar", "tar", "Tar uncompressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-compressed-tar", "tar.gz", "Tar compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-bzip-compressed-tar", "tar.bz", "Tar bzip compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-xz-compressed-tar", "tar.xz", "Tar xz compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-lzma-compressed-tar", "tar.lzma", "Tar lzma compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-lrzip-compressed-tar", "tar.lrz", "Tar lrzip compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-lz4-compressed-tar", "tar.lz4", "Tar lz4 compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-lzip-compressed-tar", "tar.lz", "Tar lzip compressed archive")},
    {MIME_TYPE_FULL_NAME_ONLY("application/x-tzo", "tar.lzop", "Tar lzop compressed archive")},
    {MIME_TYPE_NULL}
};

static const struct format_t libarchiveFormat = {
    .name = "libarchive",
    .objectSize = sizeof(struct archive_libarchive_t),
    .mime_types_rw = libarchive_mimes_rw,
    .mime_types_ro = libarchive_mimes_ro,
    .openArchive = libarchive_openArchive,
    .listFiles = libarchive_listFiles,
    .deleteFiles = libarchive_deleteFiles,
    .closeArchive = libarchive_closeArchive
};

ADD_FORMAT(libarchiveFormat);
