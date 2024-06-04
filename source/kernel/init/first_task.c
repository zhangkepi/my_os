#include "applib/lib_syscall.h"
#include "dev/tty.h"
#include "sys/_intsup.h"
#include "tools/log.h"


int first_task_main(void) {

#if 0
    int count = 3;
    int pid = get_pid();

    print_msg("first task id=%d\n", pid);

    pid = fork();
    if (pid < 0) {
        print_msg("create child proc failed.\n", 0);
    } else if (pid == 0) {
        print_msg("child: %d", count);
        char * argv[] = {"arg0", "arg1", "arg2", "arg3"};
        execve("/shell.elf", argv, (char **)0);
    } else {
        print_msg("child task id=%d\n", pid);
        print_msg("parent: %d\n", count);
    }
#endif

    for (int i = 0; i < TTY_NR; i++) {
        int pid = fork();
        if (pid < 0) {
            log_printf("create shell failed.");
            break;
        } else if (pid == 0) {
            char tty_num[5] = "tty:?";
            tty_num[4] = i + '0';
            char * argv[] = {tty_num, (char *)0};
            execve("/shell.elf", argv, (char **)0);
            while (1) {
                msleep(1000);
            }
        }
    }

    for (; ; ) {
        //print_msg("task id=%d", get_pid());
        msleep(1000);
    }
    return 0;
}