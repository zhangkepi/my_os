#ifndef TASK_H_
#define TASK_H_

#include "comm/types.h"
#include "cpu/cpu.h"

typedef struct _task_t {
    // uint32_t * stack;
    tss_t tss;
    int tss_sel;
}task_t;


int task_init(task_t * task, uint32_t entry, uint32_t esp);

void task_switch_from_to(task_t * from, task_t * to);

void simple_switch(uint32_t ** from_stack, uint32_t * to_stack);

#endif