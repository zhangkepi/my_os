#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "types.h"


// 关中断 TODO static关键字的作用？
static inline void cli(void) {
    __asm__ __volatile__("cli");
}

// 开中断
static inline void sti(void) {
    __asm__ __volatile__("sti");
}


// 从指定端口读一个字节  inb al(port), dx(data)
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb  %[p], %[v]":[v]"=a"(ret):[p]"d"(port));
    return ret;
}

// 从指定端口读两个字节  in al(port), dx(data)
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__("in  %[p], %[v]":[v]"=a"(ret):[p]"d"(port));
    return ret;
}


// 往指定端口写一个字节数据  outb al(data), dx(port)
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %[v], %[p]"::[p]"d"(port), [v]"a"(data));
}

// 加载GDT表
static inline void lgdt(uint32_t start, uint32_t size) {
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start31_16;
    }gdt;
    
    gdt.start31_16 = start >> 16;
    gdt.start15_0 = start & 0xFFFF;
    gdt.limit = size - 1;

    __asm__ __volatile__("lgdt %[g]"::[g]"m"(gdt));
}

// 加载GDT表
static inline void lidt(uint32_t start, uint32_t size) {
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start31_16;
    }idt;
    
    idt.start31_16 = start >> 16;
    idt.start15_0 = start & 0xFFFF;
    idt.limit = size - 1;

    __asm__ __volatile__("lidt %[g]"::[g]"m"(idt));
}

// 读取CRO寄存器
static inline uint32_t read_cr0(void) {
    uint32_t cr0;

    __asm__ __volatile__("mov %%cr0, %[v]":[v]"=r"(cr0));

    return cr0;
}

// 写入CR0
static inline void write_cr0(uint32_t v) {
    __asm__ __volatile__("mov %[v], %%cr0"::[v]"r"(v));
}

// 远跳转
static inline void far_jmp(uint32_t selector, uint32_t offset) {
    uint32_t addr[] = {offset, selector};
    __asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));
}

static inline void hlt(void) {
    __asm__ __volatile__("hlt");
}

static inline void write_tr(uint16_t tss_sel) {
    __asm__ __volatile__("ltr %%ax"::"a"(tss_sel));
}

static inline uint32_t read_eflags() {
    uint32_t eflags;
    __asm__ __volatile__("pushf\n\tpop %%eax":"=a"(eflags));
    return eflags;
}

static inline void write_eflags(uint32_t eflags) {
    __asm__ __volatile__("push %%eax\n\tpopf"::"a"(eflags));
}

#endif