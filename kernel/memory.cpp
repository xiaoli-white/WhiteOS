#include <stdio.h>
#include <string.h>
#include <limine.h>

#include <kernel/memory.h>

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = 0};

PMM pmm;

const char *get_memory_type_string(uint32_t type)
{
    switch (type)
    {
    case 0:
        return "USABLE";
    case 1:
        return "RESERVED";
    case 2:
        return "ACPI_RECLAIMABLE";
    case 3:
        return "ACPI_NVS";
    case 4:
        return "BAD_MEMORY";
    case 5:
        return "BOOTLOADER_RECLAIMABLE";
    case 6:
        return "KERNEL_AND_MODULES";
    case 7:
        return "FRAMEBUFFER";
    default:
        return "UNKNOWN";
    }
}

void get_memory_info()
{
    printf("\n=== Memory Information ===\n");

    if (memmap_request.response == 0)
    {
        printf("❌ No memory map response from Limine\n");
        printf("   Possible reasons:\n");
        printf("   - Limine version incompatible\n");
        printf("   - Memory map request not properly set up\n");
        printf("   - Bootloader issue\n");
        return;
    }

    struct limine_memmap_response *memmap = (struct limine_memmap_response *)memmap_request.response;

    if (memmap->entry_count == 0)
    {
        printf("❌ Memory map is empty\n");
        return;
    }

    printf("Memory map revision: %d\n", memmap->revision);
    printf("Total memory regions: %d\n\n", memmap->entry_count);

    uint64_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    printf("CR0: %p\n", cr0);
    if (cr0 & 0x1)
    {
        printf("   Protected Mode: ENABLED\n");
    }
    else
    {
        printf("   Protected Mode: DISABLED\n");
    }
    if (cr0 & 0x80000000)
    {
        printf("   Paging: ENABLED\n");
    }
    else
    {
        printf("   Paging: DISABLED\n");
    }

    printf("Memory Regions:\n");
    printf("┌─────┬─────────────────┬─────────────────┬─────────────────┬────────────┐\n");
    printf("│ No. │ Start Address   │ End Address     │ Size            │ Type       │\n");
    printf("├─────┼─────────────────┼─────────────────┼─────────────────┼────────────┤\n");

    uint64_t total_memory = 0;
    uint64_t usable_memory = 0;
    uint64_t reserved_memory = 0;
    uint64_t kernel_memory = 0;
    uint64_t bootloader_memory = 0;
    uint64_t framebuffer_memory = 0;
    uint64_t acpi_memory = 0;
    uint64_t bad_memory = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap->entries[i];
        uint64_t end_address = entry->base + entry->length - 1;
        const char *type_str = get_memory_type_string(entry->type);
        const char *size_unit = "B";
        uint64_t size_value = entry->length;

        if (entry->length >= 1024 * 1024 * 1024)
        {
            size_value = entry->length / (1024 * 1024 * 1024);
            size_unit = "GB";
        }
        else if (entry->length >= 1024 * 1024)
        {
            size_value = entry->length / (1024 * 1024);
            size_unit = "MB";
        }
        else if (entry->length >= 1024)
        {
            size_value = entry->length / 1024;
            size_unit = "KB";
        }

        printf("│ %d │ %p │ %p │ %ul %s │ %s │\n",
               i, entry->base, end_address,size_value, size_unit, type_str);

        total_memory += entry->length;

        switch (entry->type)
        {
        case LIMINE_MEMMAP_USABLE:
            usable_memory += entry->length;
            break;
        case LIMINE_MEMMAP_RESERVED:
            reserved_memory += entry->length;
            break;
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            acpi_memory += entry->length;
            break;
        case LIMINE_MEMMAP_ACPI_NVS:
            acpi_memory += entry->length;
            break;
        case LIMINE_MEMMAP_BAD_MEMORY:
            bad_memory += entry->length;
            break;
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            bootloader_memory += entry->length;
            break;
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            kernel_memory += entry->length;
            break;
        case LIMINE_MEMMAP_FRAMEBUFFER:
            framebuffer_memory += entry->length;
            break;
        }
    }

    printf("└─────┴─────────────────┴─────────────────┴─────────────────┴────────────┘\n");

    printf("\nMemory Statistics:\n");
    printf("┌──────────────────────────┬──────────────┬────────────┐\n");
    printf("│ Type                     │ Size         │ Percentage │\n");
    printf("├──────────────────────────┼──────────────┼────────────┤\n");

#define PRINT_MEM_STAT(name, value)         \
    printf("│ %s │ %d MB     │ %d%%   │\n", \
           name, value / (1024 * 1024),     \
           total_memory > 0 ? (value / total_memory * 100) : 0)

    PRINT_MEM_STAT("Total Memory", total_memory);
    PRINT_MEM_STAT("Usable Memory", usable_memory);
    PRINT_MEM_STAT("Reserved Memory", reserved_memory);
    PRINT_MEM_STAT("Kernel Memory", kernel_memory);
    PRINT_MEM_STAT("Bootloader Memory", bootloader_memory);
    PRINT_MEM_STAT("Framebuffer Memory", framebuffer_memory);
    PRINT_MEM_STAT("ACPI Memory", acpi_memory);
    PRINT_MEM_STAT("Bad Memory", bad_memory);

    printf("└──────────────────────────┴──────────────┴────────────┘\n");

    printf("\nAvailable Memory Regions (for kernel use):\n");
    printf("┌─────────────────┬─────────────────┬──────────────┐\n");
    printf("│ Start Address   │ End Address     │ Size         │\n");
    printf("├─────────────────┼─────────────────┼──────────────┤\n");

    int usable_count = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE)
        { // USABLE
            uint64_t end_address = entry->base + entry->length - 1;
            printf("│ %p │ %p │ %d MB    │\n",
                   entry->base, end_address,
                   entry->length / (1024 * 1024));
            usable_count++;
        }
    }

    if (usable_count == 0)
    {
        printf("│ No usable memory regions found!        │\n");
    }

    printf("└─────────────────┴─────────────────┴──────────────┘\n");
}

void PMM::init()
{
    struct limine_memmap_response *memmap = memmap_request.response;
    
    printf("Initializing Physical Memory Manager...\n");
    
    uint64_t highest_addr = 0;
    uint64_t bitmap_addr = 0;
    size_t usable_memory = 0;
    
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t end_addr = entry->base + entry->length;
            if (end_addr > highest_addr) {
                highest_addr = end_addr;
            }
            usable_memory += entry->length;
            
            if (entry->base >= 0x100000 && entry->length >= bitmapSize) {
                if (bitmap_addr == 0 || entry->base < bitmap_addr) {
                    bitmap_addr = entry->base;
                    memoryBase = entry->base;
                    memorySize = entry->length;
                }
            }
        }
    }
    
    totalBlocks = highest_addr / PMM_BLOCK_SIZE;
    usedBlocks = totalBlocks;
    bitmapSize = (totalBlocks + 63) / 64 * 8;
    
    printf("Memory Map:\n");
    printf("  Highest physical address: %p\n", highest_addr);
    printf("  Total usable memory: %d MB\n", usable_memory / 1024 / 1024);
    printf("  Total blocks: %d\n", totalBlocks);
    printf("  Bitmap size: %d bytes\n", bitmapSize);
    printf("  Bitmap address: %p\n", bitmap_addr);
    
    bitmap = (uint64_t*)bitmap_addr;
    memset(bitmap, 0xFF, bitmapSize);
    
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            size_t start_block = entry->base / PMM_BLOCK_SIZE;
            size_t end_block = (entry->base + entry->length) / PMM_BLOCK_SIZE;
            
            for (size_t block = start_block; block < end_block; block++) {
                clearBitmap(block);
                usedBlocks--;
            }
        }
    }
    
    size_t bitmap_start_block = bitmap_addr / PMM_BLOCK_SIZE;
    size_t bitmap_end_block = (bitmap_addr + bitmapSize + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    
    for (size_t block = bitmap_start_block; block < bitmap_end_block; block++) {
        setBitmap(block);
        usedBlocks++;
    }
     
    printf("%d %d\n", totalBlocks, usedBlocks);
    
    printf("PMM initialized successfully\n");
    printf("Free memory: %u MB\n", getFreeMemory() / 1024 / 1024);
}

size_t PMM::getFreeMemory() {
    return (totalBlocks - usedBlocks) * PMM_BLOCK_SIZE;
}
void PMM::setBitmap(size_t bit)
{
    bitmap[bit / 64] |= (1ULL << (bit % 64));
}

void PMM::clearBitmap(size_t bit)
{
    bitmap[bit / 64] &= ~(1ULL << (bit % 64));
}

int PMM::getBitmap(size_t bit)
{
    return  (bitmap[bit / 64] >> (bit % 64)) & 1;
}

PMM *getPMM()
{
    return &pmm;
}