#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdlib.h>

size_t arrlen(const char *const *arr) __attribute__((pure));

char **arrcpy(char **dst, char **src) __attribute__ ((__nonnull__ (1)));

void arrfree(char **arr) __attribute__ ((__nonnull__ (1)));

char **arrcatdup(char **arr, ...) __attribute__ ((sentinel)) __attribute__((malloc));

#endif // FUNCTIONS_H
