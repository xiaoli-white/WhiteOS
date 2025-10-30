#ifndef _WHITE_OS_DEBUG_H
#define _WHITE_OS_DEBUG_H

#include <stdint.h>
#include <stddef.h>

#define DEBUG_ERROR   0
#define DEBUG_WARN    1
#define DEBUG_INFO    2
#define DEBUG_VERBOSE 3

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_INFO
#endif

#define DEBUG(level, fmt, ...) \
    do { \
        if (level <= DEBUG_LEVEL) { \
            debug_printf("[%s] %s:%d: " fmt, \
                        debug_level_string(level), \
                        __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define DBG_ERROR(fmt, ...)   DEBUG(DEBUG_ERROR, fmt, ##__VA_ARGS__)
#define DBG_WARN(fmt, ...)    DEBUG(DEBUG_WARN, fmt, ##__VA_ARGS__)
#define DBG_INFO(fmt, ...)    DEBUG(DEBUG_INFO, fmt, ##__VA_ARGS__)
#define DBG_VERBOSE(fmt, ...) DEBUG(DEBUG_VERBOSE, fmt, ##__VA_ARGS__)

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            debug_printf("ASSERT FAILED: %s at %s:%d\n", \
                        #condition, __FILE__, __LINE__); \
            asm volatile ("cli; hlt"); \
        } \
    } while (0)

void hex_dump(const void *data, size_t size);
const char *debug_level_string(int level);
void debug_printf(const char *fmt, ...);

#endif