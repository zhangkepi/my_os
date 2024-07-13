#include "ipc/mutex.h"
#include "core/task.h"
#include "cpu/irq.h"
#include "tools/list.h"

void mutex_init(mutex_t * mutex) {
    mutex->locked_count = 0;
    mutex->owner = (task_t *)0;
    list_init(&mutex->wait_list);
}


void mutex_lock(mutex_t * mutex) {
    irq_state_t state = irq_enter_protection();

    task_t * curr_task = task_current();
    if (mutex->locked_count == 0) {
        mutex->locked_count++;
        mutex->owner = curr_task;
    } else if (mutex->owner == curr_task) {
        mutex->locked_count++;
    } else {
        task_set_block(curr_task);
        list_insert_last(&mutex->wait_list, &curr_task->wait_node);
        task_dispatch();
    }

    irq_leave_protection(state);
}


void mutex_unlock(mutex_t * mutex) {
    irq_state_t state = irq_enter_protection();

    task_t * curr_task = task_current();
    if (mutex->owner == curr_task) {
        if (--mutex->locked_count == 0) {
            mutex->owner = (task_t *)0;
            if (list_count(&mutex->wait_list) > 0) {
                list_node_t * node = list_remove_first(&mutex->wait_list);
                task_t * task = field_2_parent(node, task_t, wait_node);

                mutex->owner = task;
                mutex->locked_count = 1;

                task_set_ready(task);

                task_dispatch();
            }
        }
    }

    irq_leave_protection(state);
}

