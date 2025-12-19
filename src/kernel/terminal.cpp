#include <kernel/terminal.h>
#include <kernel/font.h>
#include <limine.h>
#include <stdarg.h>
#include <string.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static struct limine_framebuffer *framebuffer = NULL;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint32_t foreground_color = 0xFFFFFFFF; // 白色
static uint32_t background_color = 0x00000000; // 黑色

volatile struct limine_framebuffer_request* get_framebuffer_request() {
    return &framebuffer_request;
}

void terminal_initialize(void) {
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        return;
    }
    
    framebuffer = framebuffer_request.response->framebuffers[0];
    terminal_row = 0;
    terminal_column = 0;
    
    terminal_clear();
}

static void set_pixel(size_t x, size_t y, uint32_t color) {
    if (x >= framebuffer->width || y >= framebuffer->height) {
        return;
    }
    
    uint64_t pitch = framebuffer->pitch;
    uint64_t index = y * pitch + x * (framebuffer->bpp / 8);
    uint32_t *pixel = (uint32_t*)((uint8_t*)framebuffer->address + index);
    *pixel = color;
}

static void draw_char(char c, size_t x, size_t y, uint32_t fg, uint32_t bg) {
    const uint8_t *glyph = font_8x16[(unsigned char)c];
    
    for (size_t gy = 0; gy < FONT_HEIGHT; gy++) {
        uint8_t row = glyph[gy];
        for (size_t gx = 0; gx < FONT_WIDTH; gx++) {
            if (row & (1 << (7 - gx))) {
                set_pixel(x + gx, y + gy, fg);
            } else {
                set_pixel(x + gx, y + gy, bg);
            }
        }
    }
}

void terminal_clear(void) {
    if (!framebuffer) return;
    
    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            set_pixel(x, y, background_color);
        }
    }
    
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_set_color(uint32_t foreground, uint32_t background) {
    foreground_color = foreground;
    background_color = background;
}

static void scroll(void) {
    if (!framebuffer) return;
    
    size_t char_height = FONT_HEIGHT;
    size_t char_width = FONT_WIDTH;
    size_t screen_width = framebuffer->width;
    size_t screen_height = framebuffer->height;
    
    size_t rows = screen_height / char_height;
    
    if (terminal_row >= rows) {
        size_t scroll_pixels = char_height;
        size_t bytes_per_pixel = framebuffer->bpp / 8;
        
        uint8_t *dst = (uint8_t*)framebuffer->address;
        uint8_t *src = dst + scroll_pixels * framebuffer->pitch;
        size_t copy_size = (screen_height - scroll_pixels) * framebuffer->pitch;
        
        memcpy(dst, src, copy_size);
        
        for (size_t y = screen_height - scroll_pixels; y < screen_height; y++) {
            for (size_t x = 0; x < screen_width; x++) {
                set_pixel(x, y, background_color);
            }
        }
        
        terminal_row = rows - 1;
    }
}

void terminal_putchar(char c) {
    if (!framebuffer) return;
    
    size_t char_width = FONT_WIDTH;
    size_t char_height = FONT_HEIGHT;
    struct limine_framebuffer *frameb = framebuffer;
    size_t screen_width = frameb->width;
    size_t screen_height = frameb->height;
    
    switch (c) {
        case '\n':
            terminal_column = 0;
            terminal_row++;
            break;
        case '\r':
            terminal_column = 0;
            break;
        case '\t':
            terminal_column = (terminal_column + 4) & ~(4 - 1);
            break;
        case '\b':
            if (terminal_column > 0) {
                terminal_column--;
                size_t x = terminal_column * char_width;
                size_t y = terminal_row * char_height;
                for (size_t gy = 0; gy < char_height; gy++) {
                    for (size_t gx = 0; gx < char_width; gx++) {
                        set_pixel(x + gx, y + gy, background_color);
                    }
                }
            }
            break;
        default:
            if ((unsigned char)c >= 32 && (unsigned char)c <= 126) {
                size_t x = terminal_column * char_width;
                size_t y = terminal_row * char_height;
                draw_char(c, x, y, foreground_color, background_color);
                terminal_column++;
            }
            break;
    }
    
    if (terminal_column * char_width >= screen_width) {
        terminal_column = 0;
        terminal_row++;
    }
    
    if ((terminal_row + 1) * char_height >= screen_height) {
        scroll();
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_set_cursor_pos(size_t x, size_t y) {
    terminal_column = x;
    terminal_row = y;
    
    size_t max_cols = framebuffer->width / FONT_WIDTH;
    size_t max_rows = framebuffer->height / FONT_HEIGHT;
    
    if (terminal_column >= max_cols) terminal_column = max_cols - 1;
    if (terminal_row >= max_rows) terminal_row = max_rows - 1;
}

size_t terminal_get_cursor_x(void) {
    return terminal_column;
}

size_t terminal_get_cursor_y(void) {
    return terminal_row;
}

size_t terminal_get_width(void) {
    return framebuffer ? framebuffer->width / FONT_WIDTH : 0;
}

size_t terminal_get_height(void) {
    return framebuffer ? framebuffer->height / FONT_HEIGHT : 0;
}

void terminal_printf(const char* format, ...) {
    terminal_writestring(format);
}