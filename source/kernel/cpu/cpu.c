#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"


static segment_desc_t gdt_table[GDT_TABLE_SIZE];

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t * desc = gdt_table + selector / sizeof(segment_desc_t);

    if (limit > 0xFFFFF) {
        attr |= SEG_G;
        limit /= 0x1000;
    }
    

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);
    desc->base31_24 = (base >> 24) & 0xFF;
}


int gdt_alloc_desc() {

    irq_state_t state = irq_enter_protection();

    for (int i = 1; i < GDT_TABLE_SIZE; i++) {
        segment_desc_t * desc = gdt_table + i;
        if (desc->attr == 0) {
            irq_leave_protection(state);
            return i * sizeof(segment_desc_t);
        }
    }

    irq_leave_protection(state);
    return -1;
}


void gate_desc_set(gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr) {
    desc->offset15_0 = offset & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr; 
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
}


void init_gdt(void) {
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(i * sizeof(segment_desc_t), 0, 0, 0);
    }
    
    segment_desc_set(KERNEL_SELECTOR_CS, 0, 0xFFFFFFFF, SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_CODE_ER | SEG_D | SEG_G);

    segment_desc_set(KERNEL_SELECTOR_DS, 0, 0xFFFFFFFF, SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_DATA_RW | SEG_D | SEG_G);

    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}

void cpu_init(void) {

    init_gdt();
    irq_init();
}

void switch_to_tss(int tss_sel) {
    far_jmp(tss_sel, 0);
}