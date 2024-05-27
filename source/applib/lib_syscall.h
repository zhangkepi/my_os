#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "core/syscall.h"
#include "os_cfg.h"
#include "comm/types.h"


typedef struct _syscall_args_t {
    int id;
    int args0;
    int args1;
    int args2;
    int args3;
}syscall_args_t;

static inline int sys_call(syscall_args_t * args) {
    uint32_t addr[] = {0, SELECTOR_SYSCALL | 0};
    int ret;
    __asm__ __volatile__ (
        "push %[arg3]\n\t"
        "push %[arg2]\n\t"
        "push %[arg1]\n\t"
        "push %[arg0]\n\t"
        "push %[id]\n\t"
        "lcalll *(%[a])"
        :"=a"(ret):
        [arg3]"r"(args->args3), 
        [arg2]"r"(args->args2), 
        [arg1]"r"(args->args1), 
        [arg0]"r"(args->args0), 
        [id]"r"(args->id), 
        [a]"r"(addr)
    );
    return ret;
}

static inline void msleep(int ms) {
    if (ms <= 0) {
        return;
    }

    syscall_args_t args;
    args.id = SYS_sleep;
    args.args0 = ms;

    sys_call(&args);
}

static inline int get_pid() {
    syscall_args_t args;
    args.id = SYS_getpid;

    return sys_call(&args);
}

static inline void print_msg(const char * fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_printmsg;
    args.args0 = (int)fmt;
    args.args1 = arg;

    sys_call(&args);
}

#endif