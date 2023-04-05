#include "init.h"
#include "os_cfg.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "core/task.h"
#include "comm/cpu_instr.h"

void kernel_init(boot_info_t * boot_info) {

    ASSERT(boot_info->ram_region_count > 0);

    cpu_init();

    log_init();
    irq_init();
    time_init();
}

static task_t first_task;
static task_t init_task;
static uint32_t init_task_stack[1024];
void init_task_entry(void) {
    int count = 0;
    for(;;) {
        log_printf("init_task: %d", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}

void init_main(void) {

    log_printf("Kernel is running...");
    log_printf("Version: %s %s", OS_VERSION, "diyx86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    // irq_enable_global();


    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);

    write_tr(first_task.tss_sel);

    int count = 0;
    for(;;) {
        log_printf("init_main: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}