/**
 * @file lifo.c
 * @brief Implementation of the Last-In-First-Out (LIFO) scheduling policy
 * 
 * This module implements the LIFO scheduling policy where the most recently added
 * process is the next to be executed. It uses a stack data structure to manage
 * the ready processes.
 */

#include "../../headers/policies/lifo.h"
#include "../../headers/data_structures/data_structures.h"
#include <stdlib.h>

/**
 * @brief Policy structure for LIFO scheduling
 * 
 * This structure holds the internal state of the LIFO scheduler,
 * including the stack that stores the ready processes.
 */
struct Policy {
    Stack* ready_stack; 
};

/**
 * @brief Creates and initializes a new LIFO policy instance
 * 
 * @param quantum Unused parameter (kept for interface compatibility)
 * @return Policy* Pointer to the newly created policy, or NULL on failure
 */
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

/**
 * @brief Destroys a LIFO policy instance and frees all associated resources
 * 
 * @param policy Pointer to the policy to be destroyed
 */
void lifo_policy_destroy(Policy* policy) {
    if (!policy) return;
    
    stack_destroy(policy->ready_stack);
    free(policy);
}

/**
 * @brief Adds a process to the LIFO ready queue
 * 
 * @param policy The LIFO policy instance
 * @param process The process to be added to the ready queue
 */
void lifo_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;
    stack_push(policy->ready_stack, process);
}

/**
 * @brief Retrieves the next process to be executed
 * 
 * @param policy The LIFO policy instance
 * @return Process* The next process to execute, or NULL if no processes are ready
 */
Process* lifo_get_next_process(Policy* policy) {
    if (!policy || stack_is_empty(policy->ready_stack)) {
        return NULL;
    }
    return stack_pop(policy->ready_stack);
}