#include "core/memory.h"
#include "ipc/mutex.h"
#include "tools/bitmap.h"


static void addr_alloc_init(addr_alloc_t * alloc, uint8_t * bits, uint32_t start, uint32_t size, uint32_t page_size) {
    mutex_init(&alloc->mutex);
    alloc->start = start;
    alloc->size = size;
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bits, alloc->size / page_size, 0);
}

static uint32_t addr_alloc_page(addr_alloc_t * alloc, int page_count) {
    uint32_t addr = 0;
    
    mutex_lock(&alloc->mutex);
    int page_idx = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);
    if (page_idx >= 0) {
        addr = alloc->start + page_idx * alloc->page_size;
    }
    mutex_unlock(&alloc->mutex);
    return addr;
}


static void addr_free_page(addr_alloc_t * addr_alloc, uint32_t addr, uint32_t page_count) {
    mutex_lock(&alloc->mutex);

    uint32_t page_idx = (addr - addr_alloc->start) / addr_alloc->page_size;
    bitmap_set_bit(&addr_alloc->bitmap, page_idx, page_count, 0);

    mutex_unlock(&alloc->mutex);
}

void memory_init(boot_info_t * boot_info) {
    addr_alloc_t addr_alloc;
    uint8_t bits[8];

    addr_alloc_init(&addr_alloc, bits, 0x1000, 64*4096, 4096);
}
