#include "loader.h"
#include "comm/elf.h"

// LBA模式读取硬盘  https://www.yuque.com/lishutong-docs/diyx86os/knuqgt
static void read_disk(uint32_t sector, uint32_t sector_count, uint8_t * buf) {

    // 指定读取的硬盘
    outb(0x1F6, 0xE0);

    // 扇区数高8bit
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    // 起始扇区
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    // 扇区数低8bit
    outb(0x1F2, (uint8_t)sector_count);
    // 起始扇区
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    // 发送读写命令
    outb(0x1F7, 0x24);

    uint16_t * data_buf = (uint16_t *)buf;
    while (sector_count--) {
        while ((inb(0x1F7) & 0x88) != 0x8) {}

        // 一个扇区512字节，每次读两个字节
        for(int i = 0; i < SECTOR_SIZE/2; i++) {
            *data_buf++ = inw(0x1F0);
        }
        
    }
    
}


static uint32_t reload_elf_file(uint8_t * file_buffer) {
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    if ((elf_hdr->e_ident[0] != 0x7F) || (elf_hdr->e_ident[1] != 'E') 
        || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')) {
            return 0;
    }

    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        
        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; i < phdr->p_filesz; i++) {
            *dest++ = *src++;
        }
        dest = (uint8_t *)phdr->p_paddr + phdr->p_filesz;
        for (int i = 0; i < phdr->p_memsz - phdr->p_filesz; i++) {
            *dest++ = 0;
        }
    }
    return elf_hdr->e_entry;
}


static void die(int code) {
    for(;;){}
}

void load_kernel(void) {
    read_disk(100, 500, (uint8_t *)SYS_KERNEL_LOAD_ADDR);
    uint32_t kernel_entry = reload_elf_file((uint8_t *)SYS_KERNEL_LOAD_ADDR);
    if (kernel_entry == 0) {
        die(-1);
    }
    ((void (*)(boot_info_t *))kernel_entry)(&boot_info);
}