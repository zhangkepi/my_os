#include "lib_syscall.h"
#include "core/syscall.h"
#include "os_cfg.h"
#include <stdlib.h>


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

void msleep(int ms) {
    if (ms <= 0) {
        return;
    }

    syscall_args_t args;
    args.id = SYS_sleep;
    args.args0 = ms;

    sys_call(&args);
}

int get_pid() {
    syscall_args_t args;
    args.id = SYS_getpid;

    return sys_call(&args);
}

void print_msg(const char * fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_printmsg;
    args.args0 = (int)fmt;
    args.args1 = arg;

    sys_call(&args);
}

int fork(void) {
    syscall_args_t args;
    args.id = SYS_fork;

    return sys_call(&args);
}

int execve(const char * name, char * const * argv, char * const * env) {
    syscall_args_t args;
    args.id = SYS_execve;
    args.args0 = (int)name;
    args.args1 = (int)argv;
    args.args2 = (int)env;

    return sys_call(&args);
}

int yield(void) {
    syscall_args_t args;
    args.id = SYS_yield;

    return sys_call(&args);
}

int open(const char * name, int flags, ...) {
    syscall_args_t args;
    args.id = SYS_open;
    args.args0 = (int)name;
    args.args1 = (int)flags;

    return sys_call(&args);
}

int read(int file, char * ptr, int len) {
    syscall_args_t args;
    args.id = SYS_read;
    args.args0 = (int)file;
    args.args1 = (int)ptr;
    args.args2 = (int)len;

    return sys_call(&args);
}

int write(int file, char * ptr, int len) {
    syscall_args_t args;
    args.id = SYS_write;
    args.args0 = (int)file;
    args.args1 = (int)ptr;
    args.args2 = (int)len;

    return sys_call(&args);
}

int close(int file) {
    syscall_args_t args;
    args.id = SYS_close;
    args.args0 = (int)file;

    return sys_call(&args);
}

int lseek(int file, int offset, int dir) {
    syscall_args_t args;
    args.id = SYS_lseek;
    args.args0 = (int)file;
    args.args1 = (int)offset;
    args.args2 = (int)dir;

    return sys_call(&args);
}

int isatty(int file) {
    syscall_args_t args;
    args.id = SYS_isatty;
    args.args0 = (int)file;

    return sys_call(&args);
}

int fstat(int file, struct stat * stat) {
    syscall_args_t args;
    args.id = SYS_fstat;
    args.args0 = file;
    args.args1 = (int)stat;

    return sys_call(&args);
}

void * sbrk(ptrdiff_t inc) {
    syscall_args_t args;
    args.id = SYS_sbrk;
    args.args0 = (int)inc;

    return (void *)sys_call(&args);
}

int dup(int file) {
    syscall_args_t args;
    args.id = SYS_dup;
    args.args0 = (int)file;

    return sys_call(&args);
}

void _exit(int status) {
    syscall_args_t args;
    args.id = SYS_exit;
    args.args0 = (int)status;

    sys_call(&args);
    for (;;) {}
}

int wait(int * status) {
    syscall_args_t args;
    args.id = SYS_wait;
    args.args0 = (int)status;

    return sys_call(&args);
}

DIR * opendir(const char * path) {
    DIR * dir = (DIR *)malloc(sizeof(DIR));
    if (dir == (DIR *)0) {
        return (DIR *)0;
    }

    syscall_args_t args;
    args.id = SYS_opendir;
    args.args0 = (int)path;
    args.args1 = (int)dir;

    int err = sys_call(&args);
    if (err < 0) {
        free(dir);
        return (DIR *)0;
    }

    return dir;
}

struct dirent * readdir(DIR * dir) {
    syscall_args_t args;
    args.id = SYS_readdir;
    args.args0 = (int)dir;
    args.args1 = (int)&dir->dirent;

    int err = sys_call(&args);
    if (err < 0) {
        return (struct dirent *)0;
    }
    return &dir->dirent;
}

int closedir(DIR * dir) {
    syscall_args_t args;
    args.id = SYS_closedir;
    args.args0 = (int)dir;

    sys_call(&args);

    free(dir);

    return 0;
}

int ioctl(int file, int cmd, int arg0, int arg1) {
    syscall_args_t args;
    args.id = SYS_ioctl;
    args.args0 = (int)file;
    args.args1 = (int)cmd;
    args.args2 = (int)arg0;
    args.args3 = (int)arg1;

    return sys_call(&args);
}

int unlink(const char * path_name) {
    syscall_args_t args;
    args.id = SYS_unlink;
    args.args0 = (int)path_name;

    return sys_call(&args);
}
