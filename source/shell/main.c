#include "lib_syscall.h"

int main(int argc, char ** argv) {
    for (int i = 0; i < argc; i++) {
        print_msg("arg: %s", argv[i]);
    }
    fork();
    yield();
    for (;;) {
        print_msg("shell pid=%d", get_pid());
        msleep(1000);
    }
}