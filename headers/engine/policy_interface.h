#ifndef POLICY_INTERFACE_H
#define POLICY_INTERFACE_H

#include "../data_structures/process.h"

// Forward declaration for the opaque handle.
// The actual definition will be inside each policy's .c file.
// e.g., typedef struct { Queue* ready_queue; } FifoPolicy;
typedef struct Policy Policy;

/**
 * @brief Creates and initializes a new instance of a policy.
 *
 * @param quantum The time quantum for policies like Round Robin. Ignored otherwise.
 * @return A pointer to the policy's internal state (the handle), or NULL on error.
 */
Policy* policy_create(int quantum);

/**
 * @brief Frees all resources used by the policy.
 *
 * @param policy The policy handle to destroy.
 */
void policy_destroy(Policy* policy);

/**
 * @brief Adds a process to the policy's data structure.
 *        Called by the engine when a process becomes READY.
 *
 * @param policy The policy handle.
 * @param process The process that has become ready.
 */
void policy_add_process(Policy* policy, Process* process);

/**
 * @brief Selects the next process to be executed according to the policy's rules.
 *
 * @param policy The policy handle.
 * @return The process to execute, or NULL if none are ready.
 */
Process* policy_get_next_process(Policy* policy);

/**
 * @brief (Optional) Notifies the policy that a clock tick has occurred.
 *        Useful for policies like aging or multi-level feedback queues.
 *
 * @param policy The policy handle.
 */
void policy_tick(Policy* policy);


#endif // POLICY_INTERFACE_H