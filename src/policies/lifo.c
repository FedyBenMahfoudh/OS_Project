#include "lifo.h"
#include "../data_structures/stack.h"
#include <stdlib.h> 

struct Policy {
    stack* ready_stack;
};

Policy* lifo_policy_create(int quantum) {
    Policy* lifo_policy = (Policy*)malloc(sizeof(Policy));
    if (!lifo_policy) return NULL;

    lifo_policy->ready_stack = stack_create();
    if (!lifo_policy->ready_stack) {
        free(lifo_policy);
        return NULL;
    }

    return (Policy*)lifo_policy;
}

void lifo_policy_destroy(Policy* policy) {
    if (!policy) return;
    
    stack_destroy(policy->ready_stack);
    free(policy);
}

void lifo_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;

    stack_push(policy->ready_stack, process);
}

Process* lifo_get_next_process(Policy* policy) {
    if (!policy || stack_is_empty(policy->ready_stack)) {
        return NULL;
    }
    return stack_pop(policy->ready_stack);
}