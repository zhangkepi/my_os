#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "comm/types.h"


// 从端口读取两个字节  
// 指令: in ax,dx
static inline uint16_t inw(uint16_t port) {
    uint16_t rv;
    __asm__ __volatile__("in %[p], %[v]":[v]"=a"(rv):[p]"d"(port));
    return rv;
}


static inline void far_jmp(uint32_t selector, uint32_t offset) {
    uint32_t addr[] = {offset, selector};
    __asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));
}


// 读取cr0寄存器的值
static inline uint32_t read_cr0(void) {
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %[v]":[v]"=r"(cr0));
    return cr0;
}

// 向cr0寄存器写入值
static inline void write_cr0(uint32_t data) {
    __asm__ __volatile__("mov %[v], %%cr0"::[v]"r"(data):);
}


// 加载gdt表
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


// 从端口读取一个字节  
// 指令: inb al,dx
static inline uint8_t inb(uint16_t port) {
    uint8_t rv;
    __asm__ __volatile__("inb %[p], %[v]":[v]"=a"(rv):[p]"d"(port));
    return rv;
}

// 往端口写一个字节数据
// outb al,dx
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %[v], %[p]"::[p]"d"(port),[v]"a"(data));
}

// 关中断
static inline void cli(void) {
    __asm__ __volatile__("cli");
}

// 开中断
static inline void sti(void) {
    __asm__ __volatile__("sti");
}

#endif