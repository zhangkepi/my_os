#include <stdarg.h>
#include "cpu/irq.h"
#include "dev/console.h"
#include "dev/dev.h"
#include "ipc/mutex.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"


#define LOG_USE_COM     0

#define     COM1_PORT       0x3F8


static int log_dev_id;

static mutex_t mutex;

void log_init(void) {
    mutex_init(&mutex);

    log_dev_id = dev_open(DEV_TTY, 0, (void *)0);

#if LOG_USE_COM
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x3);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x3);
    outb(COM1_PORT + 2, 0xc7);
    outb(COM1_PORT + 4, 0x0F);
#endif
}

void log_printf(const char * fmt, ...) {

    char str_buff[128];
    va_list args;

    kernel_memset(str_buff, '\0', sizeof(str_buff));
    va_start(args, fmt);
    kernel_vsprintf(str_buff, fmt, args);
    va_end(args);

    mutex_lock(&mutex);

#if LOG_USE_COM
    const char * p = str_buff;
    while (*p != '\0') {
        while ((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');

#else
    // console_write(0, str_buff, kernel_strlen(str_buff));
    dev_write(log_dev_id, 0, "log:", 4);
    dev_write(log_dev_id, 0, str_buff, kernel_strlen(str_buff));
    char c = '\n';
    dev_write(log_dev_id, 0, &c, 1);
    // console_write(0, &c, 1);
#endif

    mutex_unlock(&mutex);
}

