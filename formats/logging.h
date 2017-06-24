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

#else

#define LOG3(type, prefix, format)
#define LOG4(type, prefix, format, ...)

#define LOG_e(prefix, format, ...)
#define LOG_d(prefix, format, ...)
#define LOG_i(prefix, format, ...)

#define LOG_E(prefix, format, ...)
#define LOG_D(prefix, format, ...)
#define LOG_I(prefix, format, ...)

#endif

#endif // LOGGING_H
