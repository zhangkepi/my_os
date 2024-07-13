#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "sys/_intsup.h"
#include <sys/stat.h>

typedef struct _syscall_args_t {
    int id;
    int args0;
    int args1;
    int args2;
    int args3;
}syscall_args_t;

struct dirent {
    int index;
    int type;
    char name[255];
    int size;
};

typedef struct _DIR {
    int index;
    struct dirent dirent;
}DIR;

void msleep(int ms);
int get_pid();
void print_msg(const char * fmt, int arg);
int fork(void);
int execve(const char * name, char * const * argv, char * const * env);
int yield(void);

int open(const char * name, int flags, ...);
int read(int file, char * ptr, int len);
int write(int file, char * ptr, int len);
int close(int file);
int lseek(int file, int offset, int dir);
int ioctl(int file, int cmd, int arg0, int arg1);
int unlink(const char * path_name);
int isatty(int file);
int fstat(int file, struct stat * stat);
void * sbrk(ptrdiff_t inc);

int dup(int file);
void _exit(int status);
int wait(int * status);

DIR * opendir(const char * path);
struct dirent * readdir(DIR * dir);
int closedir(DIR * dir);

#endif