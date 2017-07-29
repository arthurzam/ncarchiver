#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdlib.h>
#include <stdbool.h>

#ifndef max
#  define max(a,b) ({ __auto_type _a = (a);  __auto_type _b = (b);  _a > _b ? _a : _b; })
#endif
#ifndef min
#  define min(a,b) ({ __auto_type _a = (a);  __auto_type _b = (b);  _a < _b ? _a : _b; })
#endif

size_t arrlen(const char *const *arr) __attribute__((pure));
char **arrcpy(char **dst, char **src) __attribute__ ((__nonnull__ (1)));
void arrfree(char **arr) __attribute__ ((__nonnull__));
char **arrinsert(char **arr, size_t size, char *item) __attribute__ ((__nonnull__(3)));
void arrremove(char **arr, size_t index) __attribute__ ((__nonnull__));
char **arrcatdup(char **arr, ...) __attribute__ ((sentinel)) __attribute__((malloc));


#ifndef strstartswith
#  define strstartswith(str, prefix) (strncmp((prefix), (str), strlen((prefix))) == 0)
#endif
char *strconcat(const char *s1, const char *s2, char delim) __attribute__ ((__nonnull__)) __attribute__((malloc));

bool fileExists(const char *path) __attribute__ ((__nonnull__));

#endif // FUNCTIONS_H
