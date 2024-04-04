#include "comm/boot_info.h"
#include "loader.h"
#include "comm/elf.h"

// 从磁盘第sector扇区开始读取sector_count个扇区的数据
static void read_disk(uint32_t sector, int sector_count, uint8_t * buff) {
    // 配置要读取的磁盘，工作模式
    outb(0x1F6, 0xE0);

    // 从哪个扇区开始读，读多少个扇区
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    // 从哪个扇区开始读，读多少个扇区
    outb(0x1F2, (uint8_t)sector_count);
    outb(0x1F3, (uint8_t)sector);
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    // 发送读取命令
    outb(0x1F7, 0x24);

    uint16_t * data_buff = (uint16_t *)buff;
    while(sector_count--) {
        // 磁盘未就绪一直等待
        while ((inb(0x1F7) & 0x88) != 0x8) {}

        // 一个扇区大小512字节，每次读两个字节
        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            *data_buff++ = inw(0x1F0);
        }
    }
}

static uint32_t reload_elf_file(uint8_t * file_buffer) {
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    if ((elf_hdr->e_ident[0] != 0x7F) 
        || (elf_hdr->e_ident[1] != 'E') 
        || (elf_hdr->e_ident[2] != 'L') 
        || (elf_hdr->e_ident[3] != 'F')) {
            return 0;
    }
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }
        dest = (uint8_t *)(phdr->p_paddr + phdr->p_filesz);
        for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
            *dest = 0;
        }
    }
    return elf_hdr->e_entry;
}

static void die(int code) {
    for (; ; ) {}
}

void load_kernel() {
    // 把内核加载到1MB的位置
    read_disk(100, 500, (uint8_t *)SYS_KERNEL_LOAD_ADDR);
    uint32_t kernel_entry = reload_elf_file((uint8_t *)SYS_KERNEL_LOAD_ADDR);
    if (kernel_entry == 0) {
        die(-1);
    }
    ((void (*)(boot_info_t *))kernel_entry)(&boot_info);
}