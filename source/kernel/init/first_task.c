#include "applib/lib_syscall.h"
#include "tools/log.h"


int first_task_main(void) {
    int pid = get_pid();
    for (; ; ) {
        print_msg("task id=%d", pid);
        msleep(1000);
    }
    return 0;
}