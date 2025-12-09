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
// Comparator for Priority Policy (Higher priority value = Higher priority, earlier arrival time for tie-breaking)
int priority_comparator(Process* a, Process* b) {
    // Primary sort: higher priority value first (MaxHeap behavior)
    if (a->priority != b->priority) {
        return a->priority - b->priority; // If a->priority is higher, result is positive, a is "greater"
    }

    // Secondary sort (tie-break): earlier arrival time first
    // For MaxHeap, if a->arrival_time is earlier, it should be considered "greater"
    // to be popped first. So, b->arrival_time - a->arrival_time.
    return b->arrival_time - a->arrival_time;
}

Policy* priority_policy_create(int quantum) {
    Policy* priority_policy = (Policy*)malloc(sizeof(Policy));
    if (!priority_policy)
        return NULL;

    priority_policy->heap = max_heap_create(priority_comparator);
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
void priority_policy_add_process(Policy* policy, Process* process) {
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
Process* priority_policy_get_next_process(Policy* policy) {
    if (!policy || max_heap_is_empty(policy->heap))
        return NULL;

    return max_heap_pop(policy->heap);
}


/**
 * @brief Notifies the policy that a clock tick has occurred.
 * @param policy The policy handle (unused for this Priority implementation).
 */
void priority_policy_tick(Policy* policy) {
    // This Priority policy is non-preemptive and doesn't use aging,
    // so the tick function is a no-op.
    (void)policy;
}


/**
 * @brief Determines if the scheduler should re-evaluate who is running.
 * @param policy The policy handle (unused for this Priority implementation).
 * @param running_process The process currently on the CPU.
 * @return true only if the CPU is idle.
 */
bool priority_policy_needs_reschedule(Policy* policy, Process* running_process) {
    (void)policy;
    // For a non-preemptive priority queue, a reschedule is only needed if the CPU is free.
    return running_process == NULL;
}

/**
 * @brief Gets the time quantum for a specific process.
 * @return 0, as this Priority implementation is not quantum-based.
 */
int priority_policy_get_quantum(Policy* policy, Process* process) {
    (void)policy;
    (void)process;
    return 0; // Not applicable
}

/**
 * @brief Handles process demotion (quantum expiry).
 *        Does nothing, as this Priority implementation is not a preemptive quantum-based policy.
 */
void priority_policy_demote_process(Policy* policy, Process* process) {
    // No-op
    (void)policy;
    (void)process;
}