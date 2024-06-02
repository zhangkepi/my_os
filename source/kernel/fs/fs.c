#include "fs/fs.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "comm/boot_info.h"
#include "dev/console.h"
#include "tools/klib.h"
#include "tools/log.h"

#define     TEMP_FILE_ID        100

static uint8_t TEMP_ADDR[100 * 1024];
static uint8_t * temp_pos;


//从磁盘第sector扇区开始读取sector_count个扇区的数据
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
    while(sector_count-- > 0) {
        // 磁盘未就绪一直等待
        while ((inb(0x1F7) & 0x88) != 0x8) {}

        // 一个扇区大小512字节，每次读两个字节
        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            *data_buff++ = inw(0x1F0);
        }
    }
}


int sys_open(const char * name, int flags, ...) {
    if (name[0] == '/') {
        read_disk(5000, 80, TEMP_ADDR);
        temp_pos = TEMP_ADDR;
        return TEMP_FILE_ID;
    }
    return -1;
}


int sys_read(int file, char * ptr, int len) {
    if (file == TEMP_FILE_ID) {
        kernel_memcpy(temp_pos, ptr, len);
        temp_pos += len;
        return len;
    }
    return -1;
}


int sys_write(int file, char * ptr, int len) {
    if (file == 1) {
        console_write(0, ptr, len);
    }
    return 0;
}

int sys_lseek(int file, int ptr, int dir) {
    if (file == TEMP_FILE_ID) {
        temp_pos = TEMP_ADDR + ptr;
        return 0;
    }
    return -1;
}


int sys_close(int file) {
    return 0;
}

int sys_isatty(int file) {
    return -1;
}

int sys_fstat(int file, struct stat * st) {
    return -1;
}