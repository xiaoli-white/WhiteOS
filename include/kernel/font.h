#ifndef _WHITEOS_FONT_H
#define _WHITEOS_FONT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t font_8x16[256][16];

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#ifdef __cplusplus
}
#endif

#endif