#ifndef MEMORY_H_
#define MEMORY_H_

#include "comm/types.h"
#include "comm/boot_info.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"


typedef struct _addr_alloc_t {
    mutex_t mutex;
    bitmap_t bitmap;
    uint32_t start;
    uint32_t size;
    uint32_t page_size;
}addr_alloc_t;


void memory_init(boot_info_t * boot_info);

#endif