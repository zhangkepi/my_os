#include "core/memory.h"
#include "ipc/mutex.h"
#include "tools/bitmap.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "cpu/mmu.h"


static addr_alloc_t paddr_alloc;

static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE)));

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
    mutex_lock(&addr_alloc->mutex);

    uint32_t page_idx = (addr - addr_alloc->start) / addr_alloc->page_size;
    bitmap_set_bit(&addr_alloc->bitmap, page_idx, page_count, 0);

    mutex_unlock(&addr_alloc->mutex);
}


void show_mem_info(boot_info_t * boot_info) {
    log_printf("mem region:\r\n");
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        uint32_t start = boot_info->ram_region_cfg[i].start;
        uint32_t size = boot_info->ram_region_cfg[i].size;
        log_printf("mem region [%d]: 0x%x - 0x%x \r\n", i, start, size);
    }
}

static uint32_t total_mem_size(boot_info_t * boot_info) {
    uint32_t mem_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        mem_size += boot_info->ram_region_cfg[i].size;
    }
    return mem_size;
}


pte_t * find_pte(pde_t * page_dir, uint32_t vaddr, int alloc) {

    pte_t * page_table;
    pde_t * pde = page_dir + pde_index(vaddr);
    if (pde->present) {
        page_table = (pte_t *)pde_addr(pde);
    } else {
        if (alloc == 0) {
            return (pte_t *)0;
        }
        addr_alloc_page(&paddr_alloc, 1);
    }
}

void memory_create_map(pde_t * page_dir, uint32_t vaddr, uint32_t paddr, int page_count, uint32_t perm) {
    for (int i = 0; i < page_count; i++) {
        pte_t * pte = find_pte(page_dir, vaddr, 1);
        if (pte == (pte_t *)0) {
            return -1;
        }
        ASSERT(pte->present == 0);
        pte->v = paddr | perm | PTE_P;

        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }
    
}


void create_kernel_table(void) {
    extern uint8_t s_text[], e_text[], s_data[], kernel_base[];
    static memory_map_t kernel_map[] = {
        {kernel_base, s_text, kernel_base, 0},
        {s_text, e_text, s_text, 0},
        {s_data, (void *)MEM_EBDA_START, s_data, 0}
    };

    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
        memory_map_t * map = kernel_map + i;
        uint32_t vstart = align_down((uint32_t)map->vstart, MEM_PAGE_SIZE);
        uint32_t vend = align_up((uint32_t)map->vend, MEM_PAGE_SIZE);
        int page_count = (vend - vstart) / MEM_PAGE_SIZE;

        memory_create_map(kernel_page_dir, vstart, (uint32_t)map->pstart, page_count, map->perm);
    }
    
}

void memory_init(boot_info_t * boot_info) {

    extern uint8_t * mem_free_start;

    log_printf("mem init\r\n");
    show_mem_info(boot_info);

    uint8_t * kernel_mem_free = (uint8_t *)&mem_free_start;

    uint32_t mem_up1MB_free = total_mem_size(boot_info) - MEM_EXT_START;
    mem_up1MB_free = align_down(mem_up1MB_free, MEM_PAGE_SIZE);
    log_printf("free memory: 0x%x, size: 0x%x", MEM_EXT_START, mem_up1MB_free);
    addr_alloc_init(&paddr_alloc, kernel_mem_free, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE);
    kernel_mem_free += bitmap_byte_count(paddr_alloc.size / paddr_alloc.page_size);

    ASSERT(kernel_mem_free < (uint8_t *)MEM_EBDA_START);

    create_kernel_table();

    mmu_set_page_dir((uint32_t)kernel_page_dir);
}
