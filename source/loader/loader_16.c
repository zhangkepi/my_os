__asm__(".code16gcc");

#include "loader.h"

boot_info_t boot_info;

// 控制台输出字符串
static void show_msg(const char * msg) {
    char c;
    while ((c = *msg++) != '\0') {
        __asm__(
            "mov $0xe, %%ah\n\t"
	        "mov %[ch], %%al\n\t"
	        "int $0x10"::[ch]"r"(c)
        );
    }
}

// 检测内存
static void detect_memory(void) {
    show_msg("try to detect memory: ");

    SMAP_entry_t smap_entry;

    uint32_t countID = 0;
    int entries = 0, signature, bytes;

    boot_info.ram_region_count = 0;
    for (int i = 0; i < BOOT_RAM_REGION_MAX; i++) {

        SMAP_entry_t* entry = &smap_entry;

        // BIOS在中断读取内存信息 https://www.yuque.com/lishutong-docs/diyx86os/iovmcq
        __asm__ __volatile__(
            "int $0x15"
            :"=a"(signature), "=c"(bytes), "=b"(countID)
            :"a"(0xE820), "b"(countID), "c"(24), "d"(0x534D4150), "D"(entry)
        );

        if (signature != 0x534D4150) {
            show_msg("failed\r\n");
            return;
        }

        if (bytes > 20 && (entry->ACPI &0x0001) == 0) {
            continue;
        }
        
        if (entry->Type == 1) {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
        }
        
        if (countID == 0) {
            break;
        }
    }
    show_msg("ok\r\n");
}

uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0x0000, 0x9a00, 0x00cf},
    {0xFFFF, 0x0000, 0x9200, 0x00cf}
};

// 进入保护模式  https://www.yuque.com/lishutong-docs/diyx86os/zb5oui
static void entry_protect_mode(void) {
    
    // step1 关中断
    cli();

    // step2 打开A20地址线
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);

    // step3 加载GDT表
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    // step4 设置CR0的保护模式使能位
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));

    // step5 远跳转，清空流水线
    far_jmp(8, (uint32_t)protect_mode_entry);
}

void loader_entry(void) {
    show_msg("......loading......\r\n");
    detect_memory();
    entry_protect_mode();
    for(;;){}
}