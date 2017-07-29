#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#ifndef NDEBUG

extern FILE* loggerFile;

#define LOG3(type, prefix, format) fprintf(loggerFile, type ": " prefix ": " format "\n")
#define LOG4(type, prefix, format, ...) fprintf(loggerFile, type ": " prefix ": " format "\n", __VA_ARGS__)

#define LOG_e(prefix, format, ...) LOG3("error", prefix, format)
#define LOG_d(prefix, format, ...) LOG3("debug", prefix, format)
#define LOG_i(prefix, format, ...) LOG3("info", prefix, format)

#define LOG_E(prefix, format, ...) LOG4("error", prefix, format, __VA_ARGS__)
#define LOG_D(prefix, format, ...) LOG4("debug", prefix, format, __VA_ARGS__)
#define LOG_I(prefix, format, ...) LOG4("info", prefix, format, __VA_ARGS__)

void _nc_abort(const char *file, int line, const char *msg) __attribute__ ((__noreturn__));

#define NC_ASSERT(expr,msg) while(!(expr)) {_nc_abort(__FILE__, __LINE__, (msg)); }
#define NC_ASSERT_X(expr,msg) NC_ASSERT((expr), (msg))

#else

#define LOG3(type, prefix, format)
#define LOG4(type, prefix, format, ...)

#define LOG_e(prefix, format, ...)
#define LOG_d(prefix, format, ...)
#define LOG_i(prefix, format, ...)

#define LOG_E(prefix, format, ...)
#define LOG_D(prefix, format, ...)
#define LOG_I(prefix, format, ...)

#define NC_ASSERT(expr,msg)
#define NC_ASSERT_X(expr,msg) (void)(expr)

#endif

#define NC_ASSERT_VAL(expr,val)       NC_ASSERT((expr) == (val), #expr " != " #val)
#define NC_ASSERT_RANGE(expr,min,max) NC_ASSERT((expr) >= (min) && (expr) <= (max), #expr " not in range [" #min " , " #max "]")
#define NC_ASSERT_NONNULL(expr)       NC_ASSERT((expr) != NULL, #expr " is NULL")

#endif // LOGGING_H
