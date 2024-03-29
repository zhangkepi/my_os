#include <stdarg.h>
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"
#include "cpu/irq.h"
#include "ipc/mutex.h"


static mutex_t mutex;


#define COMM1_PORT      0x3F8

void log_init(void) {
    mutex_init(&mutex);
    // 关闭串行接口中断
    outb(COMM1_PORT + 1, 0x00);
    // 设置发送速率
    outb(COMM1_PORT + 3, 0x80);
    outb(COMM1_PORT + 0, 0x3);
    outb(COMM1_PORT + 1, 0x00);
    outb(COMM1_PORT + 3, 0x03);
    outb(COMM1_PORT + 2, 0xc7);
    outb(COMM1_PORT + 4, 0x0F);
}

void log_printf(const char * fmt, ...) {

    char str_buf[128];

    va_list args;
    
    kernel_memset(str_buf, '\0', sizeof(str_buf));
    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args);

    mutex_lock(&mutex);
    const char * p = str_buf;
    while (*p != '\0') {
        // 判断设备是否busy
        while (inb(COMM1_PORT + 5) & (1 << 6) == 0){}
        outb(COMM1_PORT, *p++);  
    }
    outb(COMM1_PORT, '\r');
    outb(COMM1_PORT, '\n'); 
    mutex_unlock(&mutex);
}