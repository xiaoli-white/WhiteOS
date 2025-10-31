#ifndef _WHITEOS_TERMINAL_H
#define _WHITEOS_TERMINAL_H

#include <stddef.h>
#include <stdint.h>
#include <limine.h>

#ifdef __cplusplus
extern "C" {
#endif

volatile struct limine_framebuffer_request* get_framebuffer_request();

void terminal_initialize(void);
void terminal_clear(void);
void terminal_set_color(uint32_t foreground, uint32_t background);

void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_printf(const char* format, ...);

void terminal_set_cursor_pos(size_t x, size_t y);
size_t terminal_get_cursor_x(void);
size_t terminal_get_cursor_y(void);

size_t terminal_get_width(void);
size_t terminal_get_height(void);

#ifdef __cplusplus
}
#endif

#endif