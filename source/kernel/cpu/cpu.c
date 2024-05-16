#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "ipc/mutex.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"

static segment_desc_t gdt_table[GDT_TABLE_SIZE];
static mutex_t mutex;

void segment_desc_set (int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t * desc = gdt_table + selector / sizeof(segment_desc_t);

    if (limit > 0xFFFFF) {
        limit /= 0x1000;
        attr |= SEG_G;
    }

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->base31_24 = (base >> 24) & 0xFF;
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);
}



void gdt_init(void) {
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i * sizeof(segment_desc_t), 0, 0, 0);
    }
    segment_desc_set(KERNEL_SELECTOR_CS, 0, 0xFFFFFFFF, 
        SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D
    );
    segment_desc_set(KERNEL_SELECTOR_DS, 0, 0xFFFFFFFF, 
        SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_D
    );
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}


void gate_desc_set(gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr) {
    desc->offset15_0 = offset & 0xFFFF;
    desc->offset31_16 = (offset >> 16);
    desc->selector = selector;
    desc->attr = attr;
}

int gdt_alloc_desc(void) {
    mutex_lock(&mutex);
    for (int i = 1; i < GDT_TABLE_SIZE; i++) {
        segment_desc_t * desc = gdt_table + i;
        if (desc->attr == 0) {
            mutex_unlock(&mutex);
            return i * sizeof(segment_desc_t);
        }
    }
    mutex_unlock(&mutex);
    return -1;
}


void cpu_init(void) {
    mutex_init(&mutex);
    gdt_init();
}

void switch_to_tss(int tss_sel) {
    far_jmp(tss_sel, 0);
}