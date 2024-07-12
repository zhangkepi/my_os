#include "getopt.h"
#include "loop/main.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char ** argv) {
    if (argc == 1) {
        char msg_buf[128];
        fgets(msg_buf, sizeof(msg_buf), stdin);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        puts(msg_buf);
        return 0;
    }

    int count = 1;
    int ch;
    while ((ch = getopt(argc, argv, "n:h")) != -1) {
        switch (ch) {
            case 'h':
                puts("echo any message");
                puts("Usage: echo [-n count] message");
                optind = 1;
                return 0;
            case 'n':
                count = atoi(optarg);
                break;
            case '?':
                if (optarg) {
                    fprintf(stderr, ESC_COLOR_ERROR"Unknown option: -%s\n"ESC_COLOR_DEFAULT, optarg);
                }
                optind = 1;
                return -1;
        }
    }
    if (optind > argc - 1) {
        fprintf(stderr, "Message is empty\n");
        optind = 1;
        return -1;
    }
    char * msg = argv[optind];
    for (int i = 0; i < count; i++) {
        puts(msg);
    }
    optind = 1;
    return 0;
}
