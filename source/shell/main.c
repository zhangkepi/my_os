#include "dev/tty.h"
#include "lib_syscall.h"
#include "sys/_intsup.h"
#include <stdio.h>

char cmd_buf[1024];

int main(int argc, char ** argv) {

#if 0
    printf("abef\b\b\b\bcd\n");     // cdef
    printf("abcd\x7f;fg\n");    // abc;fg
    printf("\0337Hello world!\0338123\n");  // 123lo world!
    printf("\033[31;42mHello,world!\033[39;49m123\n");

    printf("123\033[2DHello,word!\n");  // 光标左移2，1Hello,word!
    printf("123\033[2CHello,word!\n");  // 光标右移2，123  Hello,word!

    printf("\033[31m\n");  // ESC [pn m, Hello,world红色，其余绿色
    printf("\033[10;10H test!\n");  // 定位到10, 10，test!
    printf("\033[20;20H test!\n");  // 定位到20, 20，test!
    printf("\033[32;25;39m123\n");  // ESC [pn m, Hello,world红色，其余绿色  

    printf("\033[2J\n");
#endif

    open(argv[0], 0);
    dup(0);
    dup(0);

    // printf("Hello from shell\n");
    // printf("os version: %s\n", "1.0.0");
    // printf("%d %d %d %d\n", 1, 2, 3, 4);
    // fprintf(stderr, "error");

    printf("%s\n", argv[0]);


    for (;;) {
        gets(cmd_buf);
        puts(cmd_buf);
    }
}