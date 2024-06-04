#include "dev/tty.h"
#include "lib_syscall.h"
#include "os_cfg.h"
#include "shell/main.h"
#include "sys/_intsup.h"
#include "tools/klib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static cli_t cli;
static const char * promot = "sh #";

static void show_promot(void) {
    printf("%s", cli.promot);
    fflush(stdout);
}

static int do_help(int argc, char ** argv) {
    const cli_cmd_t * start = cli.cmd_start;
    while (start < cli.cmd_end) {
        printf("%s: %s\n", start->name, start->usage);
        start++;
    }
    return 0;
}

static int do_clear(int argc, char ** argv) {
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));
    return 0;
}

static int do_echo(int argc, char ** argv) {
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

static int do_exit(int argc, char ** argv) {
    exit(0);
    return 0;
}

static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
        .usage = "help -- list supported command",
        .do_func = do_help,
    },
    {
        .name = "clear",
        .usage = "clear -- clear screen",
        .do_func = do_clear,
    },
    {
        .name = "echo",
        .usage = "echo [-n count] msg -- echo something",
        .do_func = do_echo,
    },
    {
        .name = "quit",
        .usage = "quit from shell",
        .do_func = do_exit,
    }
};

static void cli_init(const char * promot, const cli_cmd_t * cmd_list, int size) {
    cli.promot = promot;
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + size;
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);
}

static const cli_cmd_t * find_buildin(char * name) {
    for (const cli_cmd_t * cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
    }
    return (cli_cmd_t *)0;
}

static void run_buildin(const cli_cmd_t * cmd, int argc, char ** argv) {
    int ret = cmd->do_func(argc, argv);
    if (ret < 0) {
        fprintf(stderr, ESC_COLOR_ERROR"error: %d\n"ESC_COLOR_DEFAULT, ret);
    }
}

static void run_exec_file(const char * path, int argc, char ** argv) {
    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed %s", path);
    } else if (pid == 0) {
        for (int i = 0; i < argc; i++) {
            printf("arg %d = %s\n", i, argv[i]);
        }
        exit(-1);
    } else {
        int status;
        int pid = wait(&status);
        fprintf(stderr, "cmd %s result: %d, pid=%d\n", path, status, pid);
    }
}

int main(int argc, char ** argv) {

    open(argv[0], 0);
    dup(0);
    dup(0);

    puts("Hello from x86 os");
    printf("os version: %s\n", OS_VERSION);
    puts("author: kp_zhang");
    puts("create date: 2024-06-04");

    cli_init(promot, cmd_list, sizeof(cmd_list) / sizeof(cli_cmd_t));

    for (;;) {
        show_promot();
        char * str = fgets(cli.curr_input, CLI_INPUT_SIZE, stdin);
        if (!str) {
            continue;
        }

        char * cr = strchr(cli.curr_input, '\r');
        if (cr) {
            *cr = '\0';
        }

        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];
        memset(argv, 0, sizeof(argv));
        const char * space = " ";
        char * token = strtok(cli.curr_input, space);
        while (token) {
            argv[argc++] = token;
            token = strtok(NULL, space);
        }
        if (argc == 0) {
            continue;
        }
        const cli_cmd_t * cmd = find_buildin(argv[0]);
        if (cmd) {
            run_buildin(cmd, argc, argv);
            continue;
        }
        run_exec_file("", argc, argv);
        fprintf(stderr, ESC_COLOR_ERROR"Unknown command: %s\n"ESC_COLOR_DEFAULT, cli.curr_input);
    }
}