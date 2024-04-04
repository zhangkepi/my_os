__asm__(".code16gcc");  // 以下代码按照16位实模式编译生成

#include "loader.h"

boot_info_t boot_info;

static void show_msg(const char * msg) {
    char c;
    while ((c = *msg++) != '\0') {
        // 禁止对内联汇编中的汇编语句进行优化，防止最终生成的汇编语句和预期不同
        __asm__ __volatile__ (      
            "mov $0xe, %%ah\n\t"
            "mov %[ch], %%al\n\t"
            "int $0x10"::[ch]"r"(c)
        );
    }
}

static void detect_memory(void) {
    show_msg("try to detact memory...\r\n");

    SMAP_entry_t smap_entry;

    boot_info.ram_region_count = 0;

    uint32_t countID = 0;
    int entries = 0, signature, bytes;
    for (int i = 0; i < BOOT_RAM_REGION_MAX; i++) {
        SMAP_entry_t * entry = &smap_entry;
        __asm__ __volatile__ ("int $0x15" : "=a"(signature), "=c"(bytes), "=b"(countID) : "a"(0xE820), "b"(countID), "c"(24), "d"(0x534D4150), "D"(entry));
        if(signature != 0x534D4150) {
            show_msg("detect memory failed...\r\n");
            return;
        }
        if (bytes > 20 && (entry->ACPI & 0x0001) == 0) {
            // ignore this entry
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
    show_msg("ok..\r\n");
}

uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0x0000, 0x9a00, 0x00cf},
    {0xFFFF, 0x0000, 0x9200, 0x00cf}
};

static void entry_protect_mode(void) {
    // 1. 关中断, 防止进入保护模式过程中被打断
    cli();

    // 2. 打开A20地址线
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);

    // 3. 加载gdt表
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    // 4. 开启保护位, cr0寄存器最低位置1
    uint16_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));

    // 5. 远跳转，清空cpu流水线
    far_jmp(8, (uint32_t)protect_mode_entry);
}

void loader_entry(void) {
    show_msg("...loading...\n\r");
    detect_memory();
    entry_protect_mode();
    for(;;){}
}