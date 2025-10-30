#include <kernel/debug.h>
#include <kernel/serial.h>
#include <stdarg.h>
#include <stdio.h>

void hex_dump(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t*)data;
    
    for (size_t i = 0; i < size; i += 16) {
        char line[80];
        char *ptr = line;
        
        ptr += sprintf(ptr, "%08zx: ", i);
        
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                ptr += sprintf(ptr, "%02x ", bytes[i + j]);
            } else {
                ptr += sprintf(ptr, "   ");
            }
            
            if (j == 7) {
                ptr += sprintf(ptr, " ");
            }
        }
        
        ptr += sprintf(ptr, " ");
        
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                uint8_t c = bytes[i + j];
                ptr += sprintf(ptr, "%c", (c >= 32 && c < 127) ? c : '.');
            } else {
                ptr += sprintf(ptr, " ");
            }
        }
        
        DEBUG(DEBUG_VERBOSE, "%s", line);
    }
}

const char *debug_level_string(int level) {
    switch (level) {
        case DEBUG_ERROR:   return "ERROR";
        case DEBUG_WARN:    return "WARN";
        case DEBUG_INFO:    return "INFO";
        case DEBUG_VERBOSE: return "VERBOSE";
        default: return "UNKNOWN";
    }
}

void debug_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
   
    serial_printf(fmt);
    
    va_end(args);
}