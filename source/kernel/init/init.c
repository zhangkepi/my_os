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

void test_list(void) {
    list_t list;
    list_node_t nodes[5];

    list_init(&list);

    log_printf("list: first=0x%x, last=0x%x, count=%d", list_first(&list), list_last(&list), list_count(&list));

    for (int i = 0; i < 5; i++) {
        list_node_t * node = nodes + i;
        log_printf("insert to list first: %d, 0x%x", i, (uint32_t)node);
        list_insert_first(&list, node);   
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d", list_first(&list), list_last(&list), list_count(&list));

    list_init(&list);
    for (int i = 0; i < 5; i++) {
        list_node_t * node = nodes + i;
        log_printf("insert to list first: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }

    for (int i = 0; i < 5; i++) {
        list_node_t * node = list_remove_first(&list);
        log_printf("remove first from list: %d, 0x%x", i, (uint32_t)node);
    }
    log_printf("list: first=0x%x, last=0x%x, count=%d", list_first(&list), list_last(&list), list_count(&list));


    struct type_t {
        int i;
        list_node_t node;
    }v = {0x123456};

    struct type_t * a = (struct type_t *)0;
    log_printf("a->node addr: 0x%x", &(a->node));
    log_printf("a->node from macro addr: 0x%x", offset_in_parent(struct type_t, node));
    
    
    list_node_t * v_node = &v.node;
    struct type_t * p = list_node_parent(v_node, struct type_t, node);
    log_printf("v:0x%x\r\n", &v);
    log_printf("p:0x%x\r\n", p);
    if (p->i != 0x123456) {
        log_printf("error");
    }
    
}

void init_main(void) {

    log_printf("Kernel is running...");
    log_printf("Version: %s %s", OS_VERSION, "diyx86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    // irq_enable_global();

    test_list();

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);

    write_tr(first_task.tss_sel);

    int count = 0;
    for(;;) {
        log_printf("init_main: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}