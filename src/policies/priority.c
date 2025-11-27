#include "../../headers/policies/priority.h"
#include "../../headers/data_structures/data_structures.h"
#include <stdlib.h>

/**
 * @struct Policy
 * @brief Represents the priority scheduling policy.
 *
 * The policy maintains a MaxHeap containing processes ordered by priority.
 */
struct Policy {
    MaxHeap* heap; /**< Max heap storing processes by priority. */
};

/**
 * @brief Creates a new priority scheduling policy.
 *
 * Allocates and initializes a Policy structure with an empty MaxHeap.
 *
 * @param quantum Unused parameter for this policy, kept for compatibility.
 * @return Pointer to a newly allocated Policy, or NULL on failure.
 */
Policy* priority_policy_create(int quantum) {
    Policy* priority_policy = (Policy*)malloc(sizeof(Policy));
    if (!priority_policy)
        return NULL;

    priority_policy->heap = max_heap_create();
    if (!priority_policy->heap) {
      free(priority_policy);
      return NULL;
    }

    return priority_policy;
}

/**
 * @brief Frees all memory associated with a priority policy.
 *
 * Destroys the internal max heap and frees the Policy structure itself.
 *
 * @param policy Pointer to the policy to destroy (can be NULL).
 */
void priority_policy_destroy(Policy* policy) { 
    if (!policy) return;

    max_heap_destroy(policy->heap);
    free(policy);
}

/**
 * @brief Adds a process to the priority policy.
 *
 * Inserts the process into the max heap, allowing the scheduler to
 * pick it based on its priority.
 *
 * @param policy Pointer to the priority policy.
 * @param process Process to insert into the heap.
 */
void priority_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;

    max_heap_push(policy->heap, process);
}

/**
 * @brief Retrieves and removes the next process to schedule.
 *
 * Pops the highest-priority process from the heap.
 *
 * @param policy Pointer to the priority policy.
 * @return Pointer to the next process, or NULL if the heap is empty.
 */
Process* priority_get_next_process(Policy* policy) {
    if (!policy || max_heap_is_empty(policy->heap))
        return NULL;

    return max_heap_pop(policy->heap);
}
