#ifndef SYSCALL_H
#define SYSCALL_H

#include "comm/types.h"

#define     SYS_sleep               0
#define     SYS_getpid              1
#define     SYS_printmsg            2
#define     SYSCALL_PARAM_COUNT     5

typedef struct _syscall_frame_t{
    uint32_t eflags;
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, dummy, ebx, edx, ecx, eax;
    uint32_t eip, cs;
    int func_id, arg0, arg1, arg2, arg3;
    uint32_t esp, ss;
}syscall_frame_t;

void exception_handler_syscall(void);

void do_handler_syscall(syscall_frame_t * frame);

#endif