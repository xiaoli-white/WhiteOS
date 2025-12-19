#ifndef _WHITE_OS_MEMORY_H
#define _WHITE_OS_MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PMM_BLOCK_SIZE PAGE_SIZE

#define PTE_FRAME_MASK 0x000FFFFFFFFFF000
#define PTE_FLAGS_MASK 0xFFF

enum PhysicalMemoryType
{
  MEMORY_USABLE = 1,
  MEMORY_RESERVED = 2,
  MEMORY_ACPI_RECLAIMABLE = 3,
  MEMORY_NVS = 4,
  MEMORY_BADRAM = 5
};

class PhysicalMemoryManager
{
public:
  static PhysicalMemoryManager *getInstance();

  void initialize();
  uint64_t alloc();
  uint64_t allocBlocks(size_t);
  void free(void *);
  void freeBlocks(void *, size_t);
  size_t getUsableMemory();
  size_t getFreeMemory();
  size_t getUsedMemory();

private:
  uint64_t *bitmap = nullptr;
  size_t bitmapSize = 0;
  uint64_t memoryBase = 0;
  uint64_t memorySize = 0;
  size_t totalBlocks = 0;
  size_t usableBlocks = 0;
  size_t usedBlocks = 0;
  void setBitmap(size_t);
  void clearBitmap(size_t);
  int getBitmap(size_t);
};

void getMemoryInfo(void);

enum PageTableFlags
{
  PRESENT = 1 << 0,
  WRITABLE = 1 << 1,
  USER_ACCESS = 1 << 2,
  WRITE_THROUGH = 1 << 3,
  CACHE_DISABLE = 1 << 4,
  ACCESSED = 1 << 5,
  DIRTY = 1 << 6,
  HUGE_PAGE = 1 << 7,
  GLOBAL = 1 << 8,
  NO_EXECUTE = 1ULL << 63
};
struct PageTableEntry
{
  uint64_t value;
  void set_pfn(uint64_t pfn, uint64_t flags)
  {
    value = (pfn << 12) | (flags & 0xFFF);
  }

  bool is_present() const { return value & PRESENT; }
  uint64_t get_pfn() const { return (value >> 12) & ((1ULL << 40) - 1); }
  uint64_t get_flags() const { return value & 0xFFF; }

  bool is_user() const { return value & USER_ACCESS; }
  bool is_writable() const { return value & WRITABLE; }
  bool is_dirty() const { return value & DIRTY; }
  bool is_accessed() const { return value & ACCESSED; }
};

struct PageTable
{
  PageTableEntry entries[512];

  PageTableEntry *get_entry(size_t index)
  {
    return &entries[index];
  }

  void clear()
  {
    for (size_t i = 0; i < 512; i++)
    {
      entries[i].value = 0;
    }
  }
};
class VirtualMemoryManager
{
public:
  static VirtualMemoryManager *getInstance();

  void initialize();

  void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);
  void unmap_page(uint64_t virtual_addr);
  uint64_t get_physical_address(uint64_t virtual_addr);

  void *kmalloc(size_t size);
  void kfree(void *ptr);

  bool is_mapped(uint64_t virtual_addr);
  void invalidate_tlb(uint64_t virtual_addr);

  void print_memory_map();
  void print_page_tables();

  void map_framebuffer(struct limine_framebuffer *fb);

  void *physicalToVirtual(uint64_t physical_addr);
  uint64_t virtualToPhysical(void *virtual_addr);

private:
  VirtualMemoryManager() = default;
  ~VirtualMemoryManager() = default;

  VirtualMemoryManager(const VirtualMemoryManager &) = delete;
  VirtualMemoryManager &operator=(const VirtualMemoryManager &) = delete;

  PageTable *pml4_table;

  struct HeapBlock
  {
    size_t size;
    bool used;
    HeapBlock *next;
    HeapBlock *prev;
  };

  HeapBlock *heap_start;
  uint64_t heap_current;
  uint64_t heap_end;

  PageTable *get_or_create_table(PageTableEntry *entry, uint64_t flags);
  void initialize_kernel_mappings();
  void initialize_heap();
  HeapBlock *find_free_block(size_t size);
  void merge_free_blocks();

  PageTable *clonePageTable(PageTable *table);
  void copyTable(PageTable *dst, PageTable *src);
  void copyPageTable(PageTable *src, PageTable *dst, int level);
};

inline PhysicalMemoryManager *pmm()
{
  return PhysicalMemoryManager::getInstance();
}
inline VirtualMemoryManager *vmm()
{
  return VirtualMemoryManager::getInstance();
}

#endif