#include "functions.h"

#include <stdarg.h>

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
