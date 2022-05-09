#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
#include "definitions.hpp"

void
fatal (const char* format, ...)
{
    va_list argv;
    va_start(argv, format);
    vfprintf(stderr, format, argv);
    va_end(argv);
    fputs("\n", stderr);
    exit(1);
}
