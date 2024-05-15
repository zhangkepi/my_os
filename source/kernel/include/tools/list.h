#ifndef LIST_H
#define LIST_H

#include "comm/types.h"

typedef struct _list_node_t {
    struct _list_node_t * pre;
    struct _list_node_t * next;
} list_node_t;

typedef struct _list_t {
    list_node_t * first;
    list_node_t * last;
    int count;
}list_t;


static inline void list_node_init(list_node_t * node) {
    node->pre = node->next = (list_node_t *)0;
}

static inline list_node_t * list_node_pre(list_node_t * node) {
    return node->pre;
}

static inline list_node_t * list_node_next(list_node_t * node) {
    return node->next;
}

static inline int list_is_empty(list_t * list) {
    return list->count == 0;
}

static inline int list_count(list_t * list) {
    return list->count;
}

static inline list_node_t * list_first(list_t * list) {
    return list->first;
}

static inline list_node_t * list_last(list_t * list) {
    return list->last;
}


void list_init(list_t * list);
void list_insert_first(list_t * list, list_node_t * node);
void list_insert_last(list_t * list, list_node_t * node);
list_node_t * list_remove_first(list_t * list);
list_node_t * list_remove(list_t * list, list_node_t * remove_node);


#define offset_in_parent(parent_type, filed_name)   \
    ((uint32_t)&(((parent_type *)0)->filed_name))

#define parent_addr(field_addr, parent_type, filed_name) \
    (uint32_t)((uint32_t)field_addr - offset_in_parent(parent_type, filed_name))

#define field_2_parent(field_addr, parent_type, filed_name) \
    (parent_type *)(field_addr ? parent_addr(field_addr, parent_type, filed_name) : 0)


#endif