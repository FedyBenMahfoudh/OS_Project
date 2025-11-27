#include "../../headers/policies/fifo.h"
#include "../../headers/data_structures/data_structures.h"

#include <stdlib.h>

// Define the Policy structure for FIFO
struct Policy {
    Queue* queue;
};

/**
 * Creates a new FIFO policy.
 * @param quantum Time quantum (ignored for FIFO).
 * @return A pointer to the new Policy.
 */
Policy* fifo_policy_create(int quantum) {
    Policy* fifo_policy = (Policy*) malloc(sizeof(Policy));
    if (!fifo_policy) return NULL;

    fifo_policy->queue = queue_create();
    if (!fifo_policy->queue) {
        free(fifo_policy);
        return NULL;
    }

    return fifo_policy;
}

/**
 * Destroys the FIFO policy and frees memory.
 * @param policy The policy to destroy.
 */
void fifo_policy_destroy(Policy* policy) {
    if (!policy) return;

    queue_destroy(policy->queue);
    free(policy);
}

/**
 * Adds a process to the FIFO policy.
 * @param policy The policy.
 * @param process The process to add.
 */
void fifo_add_process(Policy* policy, Process* process) {
    if ((!policy) || (!process)) return;

    queue_enqueue(policy->queue, process);
}

/**
 * Gets the next process to run from the FIFO policy.
 * @param policy The policy.
 * @return The next process to run, or NULL if none.
 */
Process* fifo_get_next_process(Policy* policy) {
    if ((!policy) || (queue_is_empty(policy->queue))) return NULL;

    return queue_dequeue(policy->queue);
}