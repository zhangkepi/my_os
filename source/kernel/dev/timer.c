#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "dev/timer.h"
#include "cpu/irq.h"
#include "os_cfg.h"


static uint32_t sys_tick;


void do_handler_time(exception_frame_t * frame) {
    sys_tick++;

    pic_send_eoi(IRQ0_TIMER);
}

static void init_pit(void) {

    uint32_t reload_count = PIT_OSC_FREQ * OS_TICKS_MS / 1000.0;    
    // 配置定时器， 使用定时器0，16位，工作在mode 3
    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNEL0 | PIT_LOAD_LOHI | PIT_MODE3);
    // 向定时器写入初始值
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF);

    irq_install(IRQ0_TIMER, (irq_handler_t)exception_handler_time);
    irq_enable(IRQ0_TIMER);
}

void time_init(void) {
    sys_tick = 0;
    init_pit();
}