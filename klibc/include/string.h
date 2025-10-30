#ifndef _STRING_H
#define _STRING_H
#include <stddef.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif 

size_t strlen(const char *str);
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
char *strcpy(char *dest, const char *src);

#ifdef __cplusplus
}
#endif

#endif
