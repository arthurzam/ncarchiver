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
    uint8_t op;
};

enum ArchiveFlags {
	ARCHIVE_ENCRYPTED = 0x1,
	ARCHIVE_READ_ONLY = 0x2
};

enum ArchiveOperation {
    OP_ADD      = 0x1,
    OP_LIST     = 0x2,
    OP_DELETE   = 0x4,
    OP_RENAME   = 0x8,
    OP_EXTRACT  = 0x10,
    OP_TEST     = 0x20,
    OP_COPY     = 0x40,
    OP_MOVE     = 0x80
};

enum ArchiveError {
    ARCHIVE_ERROR_NO = 0,
    ARCHIVE_ERROR_BAD_PASSWORD,
    ARCHIVE_ERROR_CORRUPTED,
    ARCHIVE_ERROR_FULL_DISK,
    ARCHIVE_ERROR_CANCELED
};

struct mime_type_t {
    const char *name;
    const char *extension;
    const char *prettyText;
    const char *const *compressionMethods;
    const char *const *encryptionMethods;
    uint8_t compressionLevelDefault;
    int8_t compressionLevelMax;
    int8_t compressionLevelMin;
    int8_t compressionMethodDefault;
    uint8_t encryptionMethodDefault;
};

#define MIME_TYPE_COMPRESSION_VAL(min,def,max)  .compressionLevelDefault = def, .compressionLevelMax = max, .compressionLevelMin = min
#define MIME_TYPE_COMPRESSION_NO                MIME_TYPE_COMPRESSION_VAL(-1, -1, -1)
#define MIME_TYPE_COMPRESSION_MET_VAL(list,def) .compressionMethods = list, .compressionMethodDefault = def
#define MIME_TYPE_ENCRYPTION_MET_NO             .compressionMethods = NULL, .compressionMethodDefault = -1
#define MIME_TYPE_ENCRYPTION_VAL(list,def)      .encryptionMethods = list, .encryptionMethodDefault = def
#define MIME_TYPE_ENCRYPTION_NO                 .encryptionMethods = NULL, .encryptionMethodDefault = -1
#define MIME_TYPE_FULL_NAME(n,ext,text)         .name = n, .extension = ext, .prettyText = text

#define MIME_TYPE_FULL_NAME_ONLY(n,ext,text)    MIME_TYPE_FULL_NAME(n,ext,text), MIME_TYPE_COMPRESSION_NO, MIME_TYPE_ENCRYPTION_MET_NO, MIME_TYPE_ENCRYPTION_NO
#define MIME_TYPE_NAME_ONLY(n)                  MIME_TYPE_FULL_NAME_ONLY(n,NULL,NULL)
#define MIME_TYPE_NULL MIME_TYPE_NAME_ONLY(NULL)

struct compression_options_t
{
    char *password;
    char *filename;
    char *location;
    const struct mime_type_t *mime;
    bool encryptHeaders;
    int8_t compressionLevel, compressionMethod, encryptionMethod;
};

struct format_t {
    const char *name;
    const struct mime_type_t *mime_types_rw;
    const char *const *mime_types_ro;

    bool (*openArchive)(struct archive_t *archive, char *path) __attribute__ ((__nonnull__));
    bool (*closeArchive)(struct archive_t *archive) __attribute__ ((__nonnull__));
    bool (*setPassword)(struct archive_t *archive, const char *password) __attribute__ ((__nonnull__(1)));

    struct dir_t *(*listFiles)(struct archive_t *archive) __attribute__ ((__nonnull__));
    bool (*extractFiles)(struct archive_t *archive, const char *const *files, const char *destinationFolder) __attribute__ ((__nonnull__));
    int (*deleteFiles)(struct archive_t *archive, char **files) __attribute__ ((__nonnull__));
    bool (*testFiles)(struct archive_t *archive) __attribute__ ((__nonnull__));
    bool (*addFiles)(struct archive_t *archive, const char *const *files, const struct compression_options_t *options) __attribute__ ((__nonnull__));

    size_t objectSize;
    uint8_t flags;
};

enum FormatFlags {
    FORMAT_READ_ONLY          = 0x1,
    FORMAT_ENCRYPTION         = 0x2,
    FORMAT_ENCRYPTION_HEADERS = 0x4,
    FORMAT_SINGLE_FILE        = 0x8
};

#define ADD_FORMAT(format)                            \
    static const struct format_t *ptr_##format              \
    __attribute((used, section("format_array"))) = (const struct format_t *)&format

#define section_foreach_entry(section_name, type_t, elem)     \
     for (type_t *elem =                                      \
            ({                                                \
                extern type_t __start_##section_name;         \
                &__start_##section_name;                      \
            });                                               \
            elem !=                                           \
            ({                                                \
                extern type_t __stop_##section_name;          \
                &__stop_##section_name;                       \
            });                                               \
            ++elem)

bool format_default_openArchive(struct archive_t *archive, char *path);
bool format_default_closeArchive(struct archive_t *archive);

#define TYPE_MALLOC(type) (type *)malloc(sizeof(type))
#define STRING_ARR(...) (const char *[]){__VA_ARGS__, NULL}

#endif // GLOBAL_H
