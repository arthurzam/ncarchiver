#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "logging.h"

struct dir_more_info_t {
    const char *key;
    char *value;
};

struct dir_t {
    struct dir_t *parent, *prev, *next, *subs;
    char *name;
    struct dir_more_info_t *moreInfo;
    int flags;
    int realSize, compressSize;
    int items;
};

enum DirFlags {
	NODE_SELECTED = 0x1,
    NODE_ISDIR = 0x2
};

struct format_t;

struct archive_t {
	struct dir_t *dir;
    const struct format_t *format;
	char *path;
    char *password;
    char *comment;
    const char *mime;
	uint8_t flags;
    uint8_t error;
};

enum ArchiveFlags {
	ARCHIVE_ENCRYPTED = 0x1,
	ARCHIVE_READ_ONLY = 0x2
};

enum ArchiveError {
    ARCHIVE_ERROR_NO = 0,
    ARCHIVE_ERROR_BAD_PASSWORD,
    ARCHIVE_ERROR_CORRUPTED,
    ARCHIVE_ERROR_FULL_DISK,
    ARCHIVE_ERROR_CANCELED
};

struct format_t {
    const char *name;
    const char *const *mime_types_rw;
    const char *const *mime_types_ro;

    bool (*openArchive)(struct archive_t *archive, char *path) __attribute__ ((__nonnull__));
    bool (*closeArchive)(struct archive_t *archive) __attribute__ ((__nonnull__));
    bool (*setPassword)(struct archive_t *archive, const char *password) __attribute__ ((__nonnull__(1)));

    struct dir_t *(*listFiles)(struct archive_t *archive) __attribute__ ((__nonnull__));
    bool (*extractFiles)(struct archive_t *archive, const char *const *files, const char *destinationFolder) __attribute__ ((__nonnull__));
    bool (*deleteFiles)(struct archive_t *archive, const char *const *files) __attribute__ ((__nonnull__));
    bool (*testFiles)(struct archive_t *archive) __attribute__ ((__nonnull__));

    size_t objectSize;
    uint8_t flags;
};

enum FormatFlags {
    FORMAT_READ_ONLY          = 0x1,
    FORMAT_ENCRYPTION         = 0x2,
    FORMAT_ENCRYPTION_HEADERS = 0x4,
    FORMAT_SINGLE_FILE        = 0x8
};

struct compression_options_t
{
    char *password;
    char *filename;
    char *location;
    bool encryptHeaders;
};

#define ADD_FORMAT(format)                            \
    static const struct format_t *ptr_##format              \
    __attribute((used, section("format_array"))) = (const struct format_t *)&format

bool format_default_openArchive(struct archive_t *archive, char *path);
bool format_default_closeArchive(struct archive_t *archive);

#define TYPE_MALLOC(type) (type *)malloc(sizeof(type))

#endif // GLOBAL_H
