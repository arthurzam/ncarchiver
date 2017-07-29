#include "functions.h"

#include <stdarg.h>
#include <string.h>

#include <unistd.h>

size_t arrlen(const char *const *arr) {
    if (!arr) return 0;
    size_t size;
    for (size = 0; *arr; ++size, ++arr);
    return size;
}

char **arrcpy(char **dst, char **src) {
    if (!src) return dst;
    for (; *src; ++dst, ++src)
        *dst = *src;
    return dst;
}

void arrfree(char **arr) {
    char **ptr;
    for (ptr = arr; *ptr; ++ptr)
        free(*ptr);
    free(arr);
}

char **arrinsert(char **arr, size_t size, char *item) {
    arr = (char **)realloc(arr, sizeof(char *) * (size + 2));
    arr[size++] = item;
    arr[size]   = NULL;
    return arr;
}

void arrremove(char **arr, size_t index) {
    char **ptr = arr + index;
    free(*ptr);
    for (; *ptr; ++ptr)
        *ptr = *(ptr + 1);
}

char **arrcatdup(char **arr, ...) {
    va_list args, args_size;
    char **arg = arr, **res, **ptr;
    size_t size = 1;

    va_start(args, arr);

    va_copy(args_size, args);
    while (arg != NULL) {
        size += arrlen((const char *const *)arg);
        arg = va_arg(args_size, char**);
    }
    va_end(args_size);

    ptr = res = (char **)malloc(sizeof(char *) * size);
    arg = arr;
    while (arg != NULL) {
        ptr = arrcpy(ptr, arg);
        arg = va_arg(args, char**);
    }
    *ptr = NULL;
    va_end(args);

    return res;
}


char *strconcat(const char *s1, const char *s2, char delim) {
    const unsigned s1_len = strlen(s1), s2_len = strlen(s2);
    char *res = (char *)malloc(s1_len + s2_len + 3);
    memcpy(res, s1, s1_len);
    res[s1_len] = delim;
    memcpy(res + 1 + s1_len, s2, s2_len);
    res[s1_len + 1 + s2_len] = '\0';
    return res;
}

bool fileExists(const char *path) {
    return (access(path, F_OK) == 0);
}
