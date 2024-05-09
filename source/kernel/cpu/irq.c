#include "comm/cpu_instr.h"
#include "comm/types.h"
#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "os_cfg.h"

#define     IDT_TABLE_NR    128


void exception_handler_unknown(void);

static gate_desc_t idt_table[IDT_TABLE_NR];

static void do_default_handler(exception_frame_t * frame, const char * message) {
    for (; ; ) {
        hlt();
    }
} 

void do_handler_unknown(exception_frame_t * frame) {
    do_default_handler(frame, "unknown exception");
}

void do_handler_divider(exception_frame_t * frame) {
    do_default_handler(frame, "divider zero exception");
}

void do_handler_debug(exception_frame_t * frame) {
    do_default_handler(frame, "debug zero exception");
}

void do_handler_NMI(exception_frame_t * frame) {
    do_default_handler(frame, "NMI zero exception");
}

void do_handler_breakpoint(exception_frame_t * frame) {
    do_default_handler(frame, "breakpoint zero exception");
}

void do_handler_overflow(exception_frame_t * frame) {
    do_default_handler(frame, "overflow zero exception");
}

void do_handler_bound_range(exception_frame_t * frame) {
    do_default_handler(frame, "bound_range zero exception");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
    do_default_handler(frame, "invalid_opcode zero exception");
}

void do_handler_device_unavabliable(exception_frame_t * frame) {
    do_default_handler(frame, "device_unavabliable zero exception");
}

void do_handler_double_fault(exception_frame_t * frame) {
    do_default_handler(frame, "double_fault zero exception");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
    do_default_handler(frame, "invalid_tss zero exception");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
    do_default_handler(frame, "segment_not_present zero exception");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
    do_default_handler(frame, "stack_segment_fault zero exception");
}

void do_handler_general_protection(exception_frame_t * frame) {
    do_default_handler(frame, "general_protection zero exception");
}

void do_handler_page_fault(exception_frame_t * frame) {
    do_default_handler(frame, "page_fault zero exception");
}

void do_handler_fpu_error(exception_frame_t * frame) {
    do_default_handler(frame, "fpu_error zero exception");
}

void do_handler_alignment_check(exception_frame_t * frame) {
    do_default_handler(frame, "alignment_check zero exception");
}

void do_handler_machine_check(exception_frame_t * frame) {
    do_default_handler(frame, "machine_check zero exception");
}

void do_handler_simd_excepton(exception_frame_t * frame) {
    do_default_handler(frame, "simd_excepton zero exception");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
    do_default_handler(frame, "virtual_exception zero exception");
}

void do_handler_control_exception(exception_frame_t * frame) {
    do_default_handler(frame, "control_exception zero exception");
}


int irq_install(int irq_num, irq_handler_t handler) {
    if (irq_num >= IDT_TABLE_NR) {
        return -1;
    }
    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler, GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    return 0;
}

// 初始化中断控制器芯片
void init_pic(void) {
    // 初始化主片
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_IC4);
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, PIC_ICW4_8086);

    // 初始化从片
    outb(PIC1_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_IC4);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    outb(PIC1_ICW3, 2);
    outb(PIC1_ICW4, PIC_ICW4_8086);

    // 屏蔽中断
    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xFF);
}

void irq_init(void) {
    for (int i = 0; i < IDT_TABLE_NR; i++) {
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown, GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    }
    irq_install(IRQ0_DE, (irq_handler_t)exception_handler_divider);
    irq_install(IRQ1_DB, (irq_handler_t)exception_handler_debug);
    irq_install(IRQ2_NMI, (irq_handler_t)exception_handler_NMI);
    irq_install(IRQ3_BP, (irq_handler_t)exception_handler_breakpoint);
    irq_install(IRQ4_OF, (irq_handler_t)exception_handler_overflow);
    irq_install(IRQ5_BR, (irq_handler_t)exception_handler_bound_range);
    irq_install(IRQ6_UD, (irq_handler_t)exception_handler_invalid_opcode);
    irq_install(IRQ7_NM, (irq_handler_t)exception_handler_device_unavabliable);
    irq_install(IRQ8_DF, (irq_handler_t)exception_handler_double_fault);
    irq_install(IRQ10_TS, (irq_handler_t)exception_handler_invalid_tss);
    irq_install(IRQ11_NP, (irq_handler_t)exception_handler_segment_not_present);
    irq_install(IRQ12_SS, (irq_handler_t)exception_handler_stack_segment_fault);
    irq_install(IRQ13_GP, (irq_handler_t)exception_handler_general_protection);
    irq_install(IRQ14_PF, (irq_handler_t)exception_handler_page_fault);
    irq_install(IRQ16_MF, (irq_handler_t)exception_handler_fpu_error);
    irq_install(IRQ17_AC, (irq_handler_t)exception_handler_alignment_check);
    irq_install(IRQ18_MC, (irq_handler_t)exception_handler_machine_check);
    irq_install(IRQ19_XM, (irq_handler_t)exception_handler_simd_excepton);
    irq_install(IRQ20_VE, (irq_handler_t)exception_handler_virtual_exception);
    irq_install(IRQ21_CP, (irq_handler_t)exception_handler_control_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));

    init_pic();
}


void irq_enable(int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }
    irq_num -= IRQ_PIC_START;
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
        outb(PIC0_IMR, mask);
    } else {
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable(int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }
    irq_num -= IRQ_PIC_START;
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
        outb(PIC0_IMR, mask);
    } else {
        uint8_t mask = inb(PIC1_IMR) | (1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable_global(void) {
    cli();
}

void irq_enable_global(void) {
    sti();
}

void pic_send_eoi(int irq_num) {
    irq_num -= IRQ_PIC_START;
    if (irq_num >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }
    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

