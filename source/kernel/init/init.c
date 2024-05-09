#include "init.h"
#include "cpu/irq.h"
#include "dev/time.h"

void kernel_init(boot_info_t * boot_info) {
    cpu_init();
    irq_init();
    time_init();
}

void init_main(void) {
    //int i = 3 / 0;
    //irq_enable_global();
    for (; ;) {}
}