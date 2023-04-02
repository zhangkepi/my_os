#include "init.h"
#include "os_cfg.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "tools/log.h"
#include "tools/klib.h"

void kernel_init(boot_info_t * boot_info) {

    ASSERT(boot_info->ram_region_count > 0);

    cpu_init();

    log_init();
    irq_init();
    time_init();
}


void init_main(void) {

    log_printf("Kernel is running...");
    log_printf("Version: %s %s", OS_VERSION, "diyx86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    int a = 3 / 0;
    irq_enable_global();
    for(;;){}
}