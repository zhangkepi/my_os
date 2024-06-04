#include "init.h"
#include "comm/cpu_instr.h"
#include "core/memory.h"
#include "core/task.h"
#include "cpu/irq.h"
#include "dev/console.h"
#include "dev/keyboard.h"
#include "dev/time.h"
#include "fs/fs.h"
#include "ipc/sem.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "os_cfg.h"

void kernel_init(boot_info_t * boot_info) {

    ASSERT(boot_info->ram_region_count != 0);

    cpu_init();
    irq_init();
    log_init();
    memory_init(boot_info);
    fs_init();
    time_init();
    task_manager_init();
}

void move_to_first_task(void) {
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);
    __asm__ __volatile__ (
        "push %[ss]\n\t"
        "push %[esp]\n\t"
        "push %[eflags]\n\t"
        "push %[cs]\n\t"
        "push %[eip]\n\t"
        "iret"::[ss]"r"(tss->ss), [esp]"r"(tss->esp), [eflags]"r"(tss->eflags), [cs]"r"(tss->cs), [eip]"r"(tss->eip)
    );
}

void init_main(void) {
    log_printf("Kernel is running...");
    log_printf("Version: %s %s", OS_VERSION, "diyx86 os");
    log_printf("%d %d %x %c", -123, 12345, 0x12345, 'a');

    task_first_init();

    move_to_first_task();
}

