#include "../../headers/policies/lifo.h"
#include "../../headers/data_structures/stack.h" 
#include <stdlib.h>

typedef struct {
    Stack* ready_stack;
} LifoPolicyData;


static void* lifo_create(int quantum) {
    (void)quantum; 
    LifoPolicyData* policy_data = (LifoPolicyData*)malloc(sizeof(LifoPolicyData));
    if (!policy_data) return NULL;

    policy_data->ready_stack = stack_create();
    if (!policy_data->ready_stack) {
        free(policy_data);
        return NULL;
    }
    return policy_data;
}

static void lifo_destroy(void* policy_data) {
    if (!policy_data) return;
    LifoPolicyData* lifo_data = (LifoPolicyData*)policy_data;
    stack_destroy(lifo_data->ready_stack);
    free(lifo_data);
}

static void lifo_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    LifoPolicyData* lifo_data = (LifoPolicyData*)policy_data;
    stack_push(lifo_data->ready_stack, process);
}

static Process* lifo_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    LifoPolicyData* lifo_data = (LifoPolicyData*)policy_data;
    if (stack_is_empty(lifo_data->ready_stack)) return NULL;
    return stack_pop(lifo_data->ready_stack);
}

static void lifo_tick(void* policy_data) {
    (void)policy_data;
}

static bool lifo_needs_reschedule(void* policy_data, Process* running_process) {
    (void)policy_data;
    return running_process == NULL;
}

static int lifo_get_quantum(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
    return 0; 
}

static void lifo_demote_process(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
}

static const PolicyVTable lifo_vtable = {
    .name = "lifo",
    .create = lifo_create,
    .destroy = lifo_destroy,
    .add_process = lifo_add_process,
    .get_next_process = lifo_get_next_process,
    .tick = lifo_tick,
    .needs_reschedule = lifo_needs_reschedule,
    .get_quantum = lifo_get_quantum,
    .demote_process = lifo_demote_process
};

const PolicyVTable* lifo_get_vtable() {
    return &lifo_vtable;
}