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
#include "tools/list.h"
#include "ipc/sem.h"

void kernel_init(boot_info_t * boot_info) {

    ASSERT(boot_info->ram_region_count > 0);

    cpu_init();

    log_init();
    irq_init();
    time_init();
    task_manager_init();
}


static sem_t sem;

static task_t init_task;
static uint32_t init_task_stack[1024];
void init_task_entry(void) {
    int count = 0;
    for(;;) {
        // sem_wait(&sem);
        log_printf("init_task: %d", count++);
        // sys_sleep(1000);
    }
}


void init_main(void) {

    log_printf("Kernel is running...");
    log_printf("Version: %s %s", OS_VERSION, "diyx86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    task_init(&init_task, "init_task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_first_init();

    sem_init(&sem, 0);

    irq_enable_global();

    int count = 0;
    for(;;) {
        log_printf("first_task: %d", count++);
        // sem_notify(&sem);
        // sys_sleep(1000);
    }
}