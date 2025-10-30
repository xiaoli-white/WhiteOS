#include <stdio.h>
#include <kernel/serial.h>
#include <stdarg.h>

void putchar(char c) {
    serial_putchar(c);
}

void puts(const char *str) {
    while (*str) {
        putchar(*str++);
    }
    putchar('\n');
}

static void reverse_string(char *str, size_t length) {
    size_t start = 0;
    size_t end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static size_t uint_to_string(unsigned int num, char *str, int base) {
    size_t i = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    const char *digits = "0123456789ABCDEF";
    
    while (num != 0) {
        unsigned int rem = num % base;
        str[i++] = digits[rem];
        num = num / base;
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static size_t uint64_to_string(uint64_t num, char *str, int base) {
    size_t i = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    const char *digits = "0123456789ABCDEF";
    
    while (num != 0) {
        uint64_t rem = num % base;
        str[i++] = digits[rem];
        num = num / base;
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static size_t int_to_string(int num, char *str, int base) {
    size_t i = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    int is_negative = 0;
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    
    const char *digits = "0123456789ABCDEF";
    
    while (num != 0) {
        int rem = num % base;
        str[i++] = digits[rem];
        num = num / base;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static size_t int64_to_string(int64_t num, char *str, int base) {
    size_t i = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }
    
    int is_negative = 0;
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    
    const char *digits = "0123456789ABCDEF";
    
    while (num != 0) {
        int64_t rem = num % base;
        str[i++] = digits[rem];
        num = num / base;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

int vsprintf(char *str, const char *fmt, va_list args) {
    char *ptr = str;
    char num_buf[32];
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *ptr++ = c;
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char*);
                    if (!s) s = "(null)";
                    while (*s) {
                        *ptr++ = *s++;
                    }
                    break;
                }
                case 'd': {
                    if (*(fmt+1) == 'l') {
                        fmt++;
                        if (*(fmt+1) == 'l') {
                            fmt++;
                            int64_t num = va_arg(args, int64_t);
                            size_t len = int64_to_string(num, num_buf, 10);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                        } else {
                            long num = va_arg(args, long);
                            size_t len = int_to_string(num, num_buf, 10);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                        }
                    } else {
                        int num = va_arg(args, int);
                        size_t len = int_to_string(num, num_buf, 10);
                        for (size_t i = 0; i < len; i++) {
                            *ptr++ = num_buf[i];
                        }
                    }
                    break;
                }
                case 'x': {
                    if (*(fmt+1) == 'l') {
                        fmt++;
                        if (*(fmt+1) == 'l') {
                            fmt++;
                            uint64_t num = va_arg(args, uint64_t);
                            size_t len = uint64_to_string(num, num_buf, 16);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                        } else {
                            unsigned long num = va_arg(args, unsigned long);
                            size_t len = uint_to_string(num, num_buf, 16);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                        }
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        size_t len = uint_to_string(num, num_buf, 16);
                        for (size_t i = 0; i < len; i++) {
                            *ptr++ = num_buf[i];
                        }
                    }
                    break;
                }
                case 'p': {
                    void *p = va_arg(args, void*);
                    *ptr++ = '0';
                    *ptr++ = 'x';
                    size_t len = uint64_to_string((uint64_t)(uintptr_t)p, num_buf, 16);
                    size_t padding = 16 > len ? 16 - len : 0;
                    for (size_t i = 0; i < padding; i++) {
                        *ptr++ = '0';
                    }
                    for (size_t i = 0; i < len; i++) {
                        *ptr++ = num_buf[i];
                    }
                    break;
                }
                case 'z': {
                    fmt++;
                    if (*fmt == 'x') {
                        size_t num = va_arg(args, size_t);
                        size_t len = uint64_to_string((uint64_t)num, num_buf, 16);
                        for (size_t i = 0; i < len; i++) {
                            *ptr++ = num_buf[i];
                        }
                    } else if (*fmt == 'u') {
                        size_t num = va_arg(args, size_t);
                        size_t len = uint64_to_string((uint64_t)num, num_buf, 10);
                        for (size_t i = 0; i < len; i++) {
                            *ptr++ = num_buf[i];
                        }
                    }
                    break;
                }
                case 'u':
                    if (*(fmt+1) == 'u') {
                        fmt++;
                        size_t num = va_arg(args, size_t);
                        size_t len = uint64_to_string((uint64_t)num, num_buf, 10);
                        for (size_t i = 0; i < len; i++) {
                            *ptr++ = num_buf[i];
                        }
                        break;
                    }
                    if (*(fmt+1) == 'l') {
                        fmt++;
                        if (*(fmt+1) == 'l') {
                            fmt++;
                            uint64_t num = va_arg(args, uint64_t);
                            size_t len = uint64_to_string(num, num_buf, 10);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                        } else {
                            unsigned long num = va_arg(args, unsigned long);
                            size_t len = uint_to_string(num, num_buf, 10);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                        }
                    } else {
                        unsigned int num = va_arg(args, unsigned int);
                        size_t len = uint_to_string(num, num_buf, 10);
                        for (size_t i = 0; i < len; i++) {
                            *ptr++ = num_buf[i];
                        }
                    }
                    break;
                case 'l': {
                    fmt++;
                    switch (*fmt) {
                        case 'd': {
                            long num = va_arg(args, long);
                            size_t len = int_to_string(num, num_buf, 10);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                            break;
                        }
                        case 'u': {
                            unsigned long num = va_arg(args, unsigned long);
                            size_t len = uint_to_string(num, num_buf, 10);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                            break;
                        }
                        case 'x': {
                            unsigned long num = va_arg(args, unsigned long);
                            size_t len = uint_to_string(num, num_buf, 16);
                            for (size_t i = 0; i < len; i++) {
                                *ptr++ = num_buf[i];
                            }
                            break;
                        }
                    }
                    break;
                }
                case '%': {
                    *ptr++ = '%';
                    break;
                }
                default: {
                    *ptr++ = '%';
                    *ptr++ = *fmt;
                    break;
                }
            }
        } else {
            *ptr++ = *fmt;
        }
        fmt++;
    }
    
    *ptr = '\0';
    return ptr - str;
}

int sprintf(char *str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vsprintf(str, fmt, args);
    va_end(args);
    return result;
}

int printf(const char *fmt, ...) {
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    int result = vsprintf(buffer, fmt, args);
    va_end(args);

    char *ptr = buffer;
    
     while (*ptr) {
        putchar(*ptr++);
    }
    return result;
}

int vprintf(const char *fmt, va_list args) {
    char buffer[512];
    int result = vsprintf(buffer, fmt, args);
    char *ptr = buffer;
    
    while (*ptr) {
        putchar(*ptr++);
    }
    return result;
}