/**
 * @file sjf.c
 * @brief Shortest Job First (SJF) scheduling policy implementation
 * @details This module implements the SJF scheduling algorithm, which always
 *          selects the process with the shortest burst time to execute next.
 *          This is a non-preemptive scheduling policy.
 * @author OS Project Team
 * @date 2025
 */

#include "../../headers/policies/sjf.h"
#include "../../headers/data_structures/data_structures.h"
#include <stdlib.h>

/**
 * @struct Policy
 * @brief Internal state structure for the SJF scheduling policy
 * @details This structure maintains the state of the SJF policy,
 *          including the min-heap used to efficiently select processes
 *          with the shortest burst time.
 */
struct Policy
{
    /** @brief Min-heap data structure storing ready processes sorted by burst time */
    MinHeap *Heap;
};

/**
 * @brief Comparator function for SJF (Shortest Job First).
 * @param a First process.
 * @param b Second process.
 * @return <0 if a < b, 0 if a == b, >0 if a > b (based on burst_time).
 */
int sjf_comparator(Process* a, Process* b) {
    return a->burst_time - b->burst_time;
}

/**
 * @brief Creates and initializes a new SJF policy handle
 * @param quantum Unused parameter (included for interface compatibility)
 * @return Pointer to a newly allocated Policy structure on success, NULL on failure
 * @details Allocates memory for the policy structure and initializes an internal
 *          min-heap to manage the ready processes. The heap is ordered by process
 *          burst time to efficiently select the shortest job.
 * @note The returned pointer must be freed using sjf_policy_destroy()
 * @note Errors in heap creation are properly handled with cleanup
 * @see sjf_policy_destroy
 */
Policy *sjf_policy_create(int quantum)
{
    Policy *sjf_policy = (Policy *)malloc(sizeof(Policy));

    if (!sjf_policy)
    {
        return NULL; // Échec de l'allocation mémoire
    }

    // 2. Initialiser la file interne avec le comparateur SJF
    sjf_policy->Heap = min_heap_create(sjf_comparator);
    if (!sjf_policy->Heap)
    {
        free(sjf_policy); // Nettoyage en cas d'erreur
        return NULL;
    }

    return sjf_policy;
}

/**
 * @brief Destroys and frees the resources of an SJF policy handle
 * @param policy Pointer to the Policy structure to destroy
 * @details Safely releases all memory associated with the SJF policy,
 *          including the internal min-heap and the policy structure itself.
 *          Performs NULL checks to prevent errors on invalid input.
 * @warning Calling this function with a policy twice will cause undefined behavior
 * @see sjf_policy_create
 */
void sjf_policy_destroy(Policy *policy)
{
    if (!policy)
        return;

    // Libérer la mémoire de la Heap interne
    min_heap_destroy(policy->Heap);
    // Libérer la mémoire de la politique elle-même
    free(policy);
}

/**
 * @brief Adds a process to the SJF ready queue
 * @param policy Pointer to the SJF Policy structure
 * @param process Pointer to the Process to be added
 * @details Inserts the given process into the min-heap, maintaining the heap
 *          invariant. The heap automatically orders processes by their burst time,
 *          ensuring the shortest job is always at the top.
 * @warning Does nothing if either policy or process is NULL
 * @see sjf_get_next_process
 */
void sjf_add_process(Policy *policy, Process *process)
{
    if (!policy || !process)
        return;
    // Ajouter le processus à la Heap interne
    min_heap_push(policy->Heap, process);
}

/**
 * @brief Retrieves and removes the next process to be executed
 * @param policy Pointer to the SJF Policy structure
 * @return Pointer to the Process with the shortest burst time, or NULL if no
 *         processes are available or policy is invalid
 * @details Extracts and returns the process with the minimum burst time from
 *          the ready queue. This implements the core SJF scheduling decision.
 *          The extracted process is removed from the heap.
 * @note The caller becomes responsible for the returned Process pointer
 * @see sjf_add_process
 */
Process *sjf_get_next_process(Policy *policy)
{
    if (!policy || min_heap_is_empty(policy->Heap))
    {
        return NULL;
    }
    // Récupérer le prochain processus de la Heap interne
    return min_heap_pop(policy->Heap);
}