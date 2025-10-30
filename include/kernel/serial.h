#ifndef _WHITE_OS_SERIAL_H
#define _WHITE_OS_SERIAL_H

#include <stdint.h>

#define COM1 0x3F8

#ifdef __cplusplus
extern "C" {
#endif

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char *str);
void serial_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif