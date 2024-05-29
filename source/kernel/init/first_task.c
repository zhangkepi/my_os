#include "applib/lib_syscall.h"
#include "tools/log.h"


int first_task_main(void) {
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

    for (; ; ) {
        print_msg("task id=%d", get_pid());
        msleep(1000);
    }
    return 0;
}