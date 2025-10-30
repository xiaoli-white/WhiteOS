#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

void putchar(char c);
void puts(const char *str);

int vsprintf(char *str, const char *fmt, va_list args);
int sprintf(char *str, const char *fmt, ...);
int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif