#include "applib/lib_syscall.h"
#include "dev/tty.h"
#include "sys/_intsup.h"
#include "tools/log.h"


int first_task_main(void) {

    for (int i = 0; i < 1; i++) {
        int pid = fork();
        if (pid < 0) {
            log_printf("create shell failed.");
            break;
        } else if (pid == 0) {
            char tty_num[] = "/dev/tty?";
            tty_num[sizeof(tty_num) - 2] = i + '0';
            char * argv[] = {tty_num, (char *)0};
            execve("/shell.elf", argv, (char **)0);
            while (1) {
                msleep(1000);
            }
        }
    }

    for (; ; ) {
        msleep(1000);
        int status;
        wait(&status);
    }
    return 0;
}