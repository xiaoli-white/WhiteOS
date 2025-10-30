#ifndef _WHITE_OS_MEMORY_H
#define _WHITE_OS_MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PMM_BLOCK_SIZE PAGE_SIZE

typedef enum {
    MEMORY_USABLE = 1,
    MEMORY_RESERVED = 2,
    MEMORY_ACPI_RECLAIMABLE = 3,
    MEMORY_NVS = 4,
    MEMORY_BADRAM = 5
} memory_type_t;

class PMM {
    public:
        void init();
        void* alloc();
        void free(void*);
        size_t getFreeMemory();
        private:
            uint64_t* bitmap = nullptr;
            size_t bitmapSize = 0;
            uint64_t memoryBase = 0;
            uint64_t memorySize = 0;
            size_t totalBlocks = 0;
            size_t usedBlocks = 0;
            void setBitmap(size_t);
            void clearBitmap(size_t);
            int getBitmap(size_t);
};

PMM* getPMM();
void get_memory_info(void);

#endif