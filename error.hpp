#ifndef SCRIBBLE_ERROR
#define SCRIBBLE_ERROR

#include <cstdio>
#include <cstdlib>
#include <stdarg.h>

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


#endif
