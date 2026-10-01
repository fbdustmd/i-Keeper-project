#ifndef NETSENTRY_OUTPUT_H
#define NETSENTRY_OUTPUT_H
#include <stdio.h>

/* 콘솔에는 Unicode, 파일·파이프에는 UTF-8을 출력한다. */
int output_printf(FILE *stream, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
#endif
