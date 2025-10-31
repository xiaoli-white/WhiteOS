#include <stdio.h>
#include <string.h>
#include <limine.h>

#include <kernel/memory.h>
#include <kernel/terminal.h>

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = 0};
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};
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

void getMemoryInfo()
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

  printf("Memory Regions:\n");
  printf("No.  Start Address    End Address      Size             Type\n");

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

    printf(" %d  %p  %p  %ul %s  %s\n",
           i, entry->base, end_address, size_value, size_unit, type_str);

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

  printf("\nMemory Statistics:\n");
  printf(" Type                      Size          Percentage\n");

#define PRINT_MEM_STAT(name, value)   \
  printf(" %s   %d MB       %d%%\n",  \
         name, value / (1024 * 1024), \
         total_memory > 0 ? (value / total_memory * 100) : 0)

  PRINT_MEM_STAT("Total Memory", total_memory);
  PRINT_MEM_STAT("Usable Memory", usable_memory);
  PRINT_MEM_STAT("Reserved Memory", reserved_memory);
  PRINT_MEM_STAT("Kernel Memory", kernel_memory);
  PRINT_MEM_STAT("Bootloader Memory", bootloader_memory);
  PRINT_MEM_STAT("Framebuffer Memory", framebuffer_memory);
  PRINT_MEM_STAT("ACPI Memory", acpi_memory);
  PRINT_MEM_STAT("Bad Memory", bad_memory);

  printf("\nAvailable Memory Regions (for kernel use):\n");
  printf("  Start Address     End Address       Size\n");

  int usable_count = 0;
  for (uint64_t i = 0; i < memmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE)
    { // USABLE
      uint64_t end_address = entry->base + entry->length - 1;
      printf("  %p   %p   %d MB\n",
             entry->base, end_address,
             entry->length / (1024 * 1024));
      usable_count++;
    }
  }

  if (usable_count == 0)
  {
    printf("No usable memory regions found!\n");
  }
}

PhysicalMemoryManager *PhysicalMemoryManager::getInstance()
{
  static PhysicalMemoryManager instance;
  return &instance;
}
void PhysicalMemoryManager::initialize()
{
  struct limine_memmap_response *memmap = memmap_request.response;

  printf("Initializing Physical Memory Manager...\n");

  uint64_t highest_addr = 0;
  uint64_t bitmap_addr = 0;
  size_t usable_memory = 0;

  for (size_t i = 0; i < memmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE)
    {
      uint64_t end_addr = entry->base + entry->length;
      if (end_addr > highest_addr)
      {
        highest_addr = end_addr;
      }
      usable_memory += entry->length;

      if (entry->base >= 0x100000 && entry->length >= bitmapSize)
      {
        if (bitmap_addr == 0 || entry->base < bitmap_addr)
        {
          bitmap_addr = entry->base;
          memoryBase = entry->base;
          memorySize = entry->length;
        }
      }
    }
  }

  totalBlocks = highest_addr / PMM_BLOCK_SIZE;
  bitmapSize = (totalBlocks + 63) / 64 * 8;

  bitmap = (uint64_t *)bitmap_addr;
  memset(bitmap, 0xFF, bitmapSize);

  for (size_t i = 0; i < memmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE)
    {
      size_t start_block = entry->base / PMM_BLOCK_SIZE;
      size_t end_block = (entry->base + entry->length) / PMM_BLOCK_SIZE;

      for (size_t block = start_block; block < end_block; block++)
      {
        clearBitmap(block);
        usableBlocks++;
      }
    }
  }

  size_t bitmap_start_block = bitmap_addr / PMM_BLOCK_SIZE;
  size_t bitmap_end_block = (bitmap_addr + bitmapSize + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;

  for (size_t block = bitmap_start_block; block < bitmap_end_block; block++)
  {
    setBitmap(block);
    usedBlocks++;
  }

  printf("PMM initialized successfully\n");
  printf("Memory Map:\n");
  printf("  Highest physical address: %p\n", highest_addr);
  printf("  Total usable memory: %d MB\n", usable_memory / 1024 / 1024);
  printf("  Total blocks: %d\n", totalBlocks);
  printf("  Usable blocks: %d\n", usableBlocks);
  printf("  Bitmap size: %d bytes\n", bitmapSize);
  printf("  Bitmap address: %p\n", bitmap_addr);
  printf("  Free memory: %u MB\n", getFreeMemory() / 1024 / 1024);
}

void *PhysicalMemoryManager::alloc()
{
  return allocBlocks(1);
}

void *PhysicalMemoryManager::allocBlocks(size_t blocks)
{
  if (blocks == 0)
    return NULL;

  size_t start_block = 0;
  size_t consecutive_blocks = 0;

  for (size_t i = 0; i < totalBlocks; i++)
  {
    if (!getBitmap(i))
    {
      if (consecutive_blocks == 0)
      {
        start_block = i;
      }
      consecutive_blocks++;

      if (consecutive_blocks == blocks)
      {
        for (size_t j = start_block; j < start_block + blocks; j++)
        {
          setBitmap(j);
          usedBlocks++;
        }
        return (void *)(start_block * PMM_BLOCK_SIZE);
      }
    }
    else
    {
      consecutive_blocks = 0;
    }
  }

  printf("PMM: Out of memory! Requested %lu blocks\n", blocks);
  return nullptr;
}

void PhysicalMemoryManager::free(void *ptr)
{
  freeBlocks(ptr, 1);
}

void PhysicalMemoryManager::freeBlocks(void *ptr, size_t blocks)
{
  if (ptr == NULL || blocks == 0)
    return;

  uint64_t block = (uint64_t)ptr / PMM_BLOCK_SIZE;

  for (size_t i = 0; i < blocks; i++)
  {
    if (getBitmap(block + i))
    {
      clearBitmap(block + i);
      usedBlocks--;
    }
    else
    {
      printf("PMM: Double free detected at block %lu\n", block + i);
    }
  }
}

size_t PhysicalMemoryManager::getUsableMemory()
{
  return usableBlocks * PMM_BLOCK_SIZE;
}

size_t PhysicalMemoryManager::getFreeMemory()
{
  return (usableBlocks - usedBlocks) * PMM_BLOCK_SIZE;
}
size_t PhysicalMemoryManager::getUsedMemory()
{
  return usedBlocks * PMM_BLOCK_SIZE;
}
void PhysicalMemoryManager::setBitmap(size_t bit)
{
  bitmap[bit / 64] |= (1ULL << (bit % 64));
}

void PhysicalMemoryManager::clearBitmap(size_t bit)
{
  bitmap[bit / 64] &= ~(1ULL << (bit % 64));
}

int PhysicalMemoryManager::getBitmap(size_t bit)
{
  return (bitmap[bit / 64] >> (bit % 64)) & 1;
}

VirtualMemoryManager *VirtualMemoryManager::getInstance()
{
  static VirtualMemoryManager instance;
  return &instance;
}

VirtualMemoryManager::PageTable *VirtualMemoryManager::get_or_create_table(PageTableEntry *entry, uint64_t flags)
{
  if (entry->is_present())
  {
    return (PageTable *)(entry->get_address());
  }

  void *physical_addr = pmm()->alloc();
  if (!physical_addr)
  {
    return nullptr;
  }

  PageTable *table = (PageTable *)physical_addr;
  table->clear();

  entry->set(reinterpret_cast<uint64_t>(physical_addr), flags | PRESENT | WRITABLE);
  return table;
}

void VirtualMemoryManager::initialize()
{
  printf("VMM: Initializing virtual memory...\n");

  void *pml4_physical = pmm()->alloc();
  pml4_table = (PageTable *)pml4_physical;
  pml4_table->clear();

  initialize_kernel_mappings();

  initialize_heap();

  uint64_t rip;
  asm volatile("lea (%%rip), %0" : "=r"(rip));
  printf("RIP: %p, pml4: %p\n", rip, pml4_physical);

  asm volatile("mov %0, %%cr3" ::"r"(pml4_physical));

  printf("VMM: Virtual memory initialized\n");
}

void VirtualMemoryManager::initialize_kernel_mappings()
{
  struct limine_memmap_response *memmap = memmap_request.response;
  struct limine_hhdm_response *hhdm = hhdm_request.response;

  for (size_t i = 0; i < memmap->entry_count; i++)
  {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
    {
      uint64_t physical_base = entry->base;
      uint64_t virtual_base = 0xFFFFFFFF80000000 + (physical_base & 0xFFFFFFFF);
      uint64_t pages = (entry->length + PAGE_SIZE - 1) / PAGE_SIZE;
      printf ("VMM: Mapping kernel region: %p - %p to %p - %p\n", physical_base, physical_base + entry->length, virtual_base, virtual_base + entry->length);

      for (uint64_t j = 0; j < pages; j++)
      {
        uint64_t phys_addr = physical_base + j * PAGE_SIZE;
        uint64_t virt_addr = virtual_base + j * PAGE_SIZE;

        map_page(phys_addr, virt_addr, PRESENT | WRITABLE | GLOBAL);
      }
    }
  }

  for (uint64_t i = 0; i < 1024; i++)
  {                                                // 映射 1024 个 2MB 页 = 2GB
    uint64_t phys_addr = i * 2 * 1024 * 1024;      // 2MB 页
    uint64_t virt_addr = phys_addr + hhdm->offset; // HHDM 映射

    map_page(phys_addr, virt_addr,
             PRESENT | WRITABLE | HUGE_PAGE | GLOBAL);
  }

  volatile struct limine_framebuffer_request *framebuffer_request = get_framebuffer_request();

  if (framebuffer_request->response && framebuffer_request->response->framebuffer_count > 0)
  {
    struct limine_framebuffer_response *response = (struct limine_framebuffer_response *)framebuffer_request->response;
    for (size_t i = 0; i < response->framebuffer_count; i++)
    {
      map_framebuffer(response->framebuffers[i]);
    }
  }
  printf("VMM: Kernel mappings created\n");
}

void VirtualMemoryManager::map_framebuffer(struct limine_framebuffer *fb)
{
  if (!fb)
    return;

  uint64_t fb_start = (uint64_t)fb->address;
  uint64_t fb_size = fb->height * fb->pitch;
  uint64_t fb_end = fb_start + fb_size;

  fb_end = (fb_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  for (uint64_t addr = fb_start; addr < fb_end; addr += PAGE_SIZE)
  {
    map_page(addr, addr, PRESENT | WRITABLE | NO_EXECUTE);
  }

  printf("VMM: Framebuffer mapped: %p - %p\n",
         fb_start, fb_end);
}

void VirtualMemoryManager::initialize_heap()
{
  const size_t heap_size = 16 * 1024 * 1024;
  void *heap_physical = pmm()->allocBlocks(heap_size / PAGE_SIZE);

  if (!heap_physical)
  {
    printf("VMM: Failed to allocate kernel heap!\n");
    return;
  }

  uint64_t heap_virtual = 0xFFFFFFFF82000000;

  printf("VMM: Kernel heap created at %p\n", heap_virtual);

  for (size_t i = 0; i < heap_size; i += PAGE_SIZE)
  {
    map_page(heap_virtual + i, reinterpret_cast<uint64_t>(heap_physical) + i,
             PRESENT | WRITABLE | NO_EXECUTE);
  }

  HeapBlock *heapBlock = (HeapBlock *)heap_physical;
  heapBlock->size = heap_size - sizeof(HeapBlock);
  heapBlock->used = false;
  heapBlock->next = nullptr;
  heapBlock->prev = nullptr;
  heap_start = (HeapBlock *)heap_virtual;

  heap_current = heap_virtual + sizeof(HeapBlock);
  heap_end = heap_virtual + heap_size;

  printf("VMM: Kernel heap initialized at %p\n", heap_virtual);
}

void VirtualMemoryManager::map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags)
{
  size_t pml4_index = (virtual_addr >> 39) & 0x1FF;
  size_t pdp_index = (virtual_addr >> 30) & 0x1FF;
  size_t pd_index = (virtual_addr >> 21) & 0x1FF;
  size_t pt_index = (virtual_addr >> 12) & 0x1FF;

  PageTableEntry *pml4_entry = pml4_table->get_entry(pml4_index);
  PageTable *pdp_table = get_or_create_table(pml4_entry, flags);
  if (!pdp_table)
    return;

  PageTableEntry *pdp_entry = pdp_table->get_entry(pdp_index);
  PageTable *pd_table = get_or_create_table(pdp_entry, flags);
  if (!pd_table)
    return;

  PageTableEntry *pd_entry = pd_table->get_entry(pd_index);
  PageTable *pt_table = get_or_create_table(pd_entry, flags);
  if (!pt_table)
    return;

  PageTableEntry *pt_entry = pt_table->get_entry(pt_index);
  pt_entry->set(physical_addr, flags);

  invalidate_tlb(virtual_addr);
}

void VirtualMemoryManager::unmap_page(uint64_t virtual_addr)
{
  size_t pml4_index = (virtual_addr >> 39) & 0x1FF;
  size_t pdp_index = (virtual_addr >> 30) & 0x1FF;
  size_t pd_index = (virtual_addr >> 21) & 0x1FF;
  size_t pt_index = (virtual_addr >> 12) & 0x1FF;

  PageTableEntry *pml4_entry = pml4_table->get_entry(pml4_index);
  if (!pml4_entry->is_present())
    return;

  PageTable *pdp_table = (PageTable *)(pml4_entry->get_address());
  PageTableEntry *pdp_entry = pdp_table->get_entry(pdp_index);
  if (!pdp_entry->is_present())
    return;

  PageTable *pd_table = (PageTable *)(pdp_entry->get_address());
  PageTableEntry *pd_entry = pd_table->get_entry(pd_index);
  if (!pd_entry->is_present())
    return;

  PageTable *pt_table = (PageTable *)(pd_entry->get_address());
  PageTableEntry *pt_entry = pt_table->get_entry(pt_index);

  if (pt_entry->is_present())
  {
    pmm()->free(reinterpret_cast<void *>(pt_entry->get_address()));
    pt_entry->value = 0;
    invalidate_tlb(virtual_addr);
  }
}

uint64_t VirtualMemoryManager::get_physical_address(uint64_t virtual_addr)
{
  size_t pml4_index = (virtual_addr >> 39) & 0x1FF;
  size_t pdp_index = (virtual_addr >> 30) & 0x1FF;
  size_t pd_index = (virtual_addr >> 21) & 0x1FF;
  size_t pt_index = (virtual_addr >> 12) & 0x1FF;

  PageTableEntry *pml4_entry = pml4_table->get_entry(pml4_index);
  if (!pml4_entry->is_present())
    return 0;

  PageTable *pdp_table = (PageTable *)(pml4_entry->get_address());
  PageTableEntry *pdp_entry = pdp_table->get_entry(pdp_index);
  if (!pdp_entry->is_present())
    return 0;

  PageTable *pd_table = (PageTable *)(pdp_entry->get_address());
  PageTableEntry *pd_entry = pd_table->get_entry(pd_index);
  if (!pd_entry->is_present())
    return 0;

  PageTable *pt_table = (PageTable *)(pd_entry->get_address());
  PageTableEntry *pt_entry = pt_table->get_entry(pt_index);

  if (pt_entry->is_present())
  {
    return pt_entry->get_address() + (virtual_addr & 0xFFF);
  }

  return 0;
}

bool VirtualMemoryManager::is_mapped(uint64_t virtual_addr)
{
  return get_physical_address(virtual_addr) != 0;
}

void VirtualMemoryManager::invalidate_tlb(uint64_t virtual_addr)
{
  asm volatile("invlpg (%0)" ::"r"(virtual_addr) : "memory");
}

VirtualMemoryManager::HeapBlock *VirtualMemoryManager::find_free_block(size_t size)
{
  HeapBlock *current = heap_start;

  while (current)
  {
    if (!current->used && current->size >= size)
    {
      return current;
    }
    current = current->next;
  }
  return nullptr;
}

void *VirtualMemoryManager::kmalloc(size_t size)
{
  size = (size + 7) & ~7;

  HeapBlock *block = find_free_block(size);
  if (!block)
  {
    printf("VMM: Out of heap memory!\n");
    return nullptr;
  }

  if (block->size > size + sizeof(HeapBlock) + 8)
  {
    HeapBlock *new_block = (HeapBlock *)((uint8_t *)block + sizeof(HeapBlock) + size);
    new_block->size = block->size - size - sizeof(HeapBlock);
    new_block->used = false;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next)
    {
      block->next->prev = new_block;
    }

    block->size = size;
    block->next = new_block;
  }

  block->used = true;
  return (void *)((uint8_t *)block + sizeof(HeapBlock));
}

void VirtualMemoryManager::kfree(void *ptr)
{
  if (!ptr)
    return;

  HeapBlock *block = (HeapBlock *)((uint8_t *)ptr - sizeof(HeapBlock));
  block->used = false;

  merge_free_blocks();
}

void VirtualMemoryManager::merge_free_blocks()
{
  HeapBlock *current = heap_start;

  while (current && current->next)
  {
    if (!current->used && !current->next->used)
    {
      current->size += sizeof(HeapBlock) + current->next->size;
      current->next = current->next->next;

      if (current->next)
      {
        current->next->prev = current;
      }
    }
    else
    {
      current = current->next;
    }
  }
}

void VirtualMemoryManager::print_memory_map()
{
  printf("\n=== Virtual Memory Map ===\n");

  printf("PML4 Table: 0x%llx\n", (uint64_t)pml4_table);
  printf("Kernel Heap: 0x%llx - 0x%llx\n",
         (uint64_t)heap_start, heap_end);
}

void VirtualMemoryManager::print_page_tables()
{
  printf("\n=== Page Tables ===\n");

  for (size_t i = 0; i < 512; i++)
  {
    if (pml4_table->entries[i].is_present())
    {
      printf("PML4[%lu]: 0x%llx\n", i, pml4_table->entries[i].get_address());
    }
  }
}