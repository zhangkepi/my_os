#include "ipc/mutex.h"
#include "cpu/irq.h"

void mutex_init(mutex_t * mutex) {
    mutex->locked_count = 0;
    mutex->owner = (task_t *)0;
    list_init(&mutex->wait_list);
}


void mutex_lock(mutex_t * mutex) {

    irq_state_t state = irq_enter_protection();

    task_t * curr = task_current();
    if (mutex->locked_count == 0) {
        mutex->owner = curr;
        mutex->locked_count++;
    } else if (mutex->owner == curr) {
        mutex->locked_count++;
    } else {
        task_set_block(curr);
        list_insert_last(&mutex->wait_list, &curr->wait_node);
        task_dispatch();
    }
    irq_leave_protection(state);
}


void mutex_unlock(mutex_t * mutex) {

    irq_state_t state = irq_enter_protection();
    task_t * curr = task_current();
    if (mutex->owner == curr) {
        if (--mutex->locked_count == 0) {
            mutex->owner = (task_t *)0;
            if (list_count(&mutex->wait_list) > 0) {
                list_node_t * wait_node = list_remove_first(&mutex->wait_list);
                task_t * first_wait_task = list_node_parent(wait_node, task_t, wait_node);
                mutex->owner = first_wait_task;
                mutex->locked_count = 1;
                task_set_ready(first_wait_task);
                task_dispatch();   
            }
        }
    }
    irq_leave_protection(state);
}
