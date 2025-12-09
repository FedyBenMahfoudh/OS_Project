#ifndef POLICY_INTERFACE_H
#define POLICY_INTERFACE_H

#include "../data_structures/process.h"

// Enum to identify the type of concrete policy (still needed for dispatching)
typedef enum {
    POLICY_TYPE_NONE = 0,
    POLICY_TYPE_FIFO,
    POLICY_TYPE_LIFO,
    POLICY_TYPE_SJF,
    POLICY_TYPE_PRIORITY,
    POLICY_TYPE_RR,
    POLICY_TYPE_SRT
    // Add other policy types here as they are implemented
} PolicyType;

// Forward declaration for the opaque Policy handle.
// The actual definition will be internal to src/engine/policy_interface.c
// and also internal to each concrete policy's .c file.
typedef struct Policy Policy;

/**
 * @brief Creates and initializes a new instance of a policy based on its name.
 * @param policy_name The name of the policy to create (e.g., "fifo").
 * @param quantum The time quantum for policies like Round Robin. Ignored otherwise.
 * @return A pointer to the policy's internal state (the handle), or NULL on error.
 */
Policy* policy_create(const char* policy_name, int quantum);

/**
 * @brief Frees all resources used by the policy.
 * @param policy The policy handle to destroy.
 */
void policy_destroy(Policy* policy);

/**
 * @brief Adds a process to the policy's data structure.
 * @param policy The policy handle.
 * @param process The process that has become ready.
 */
void policy_add_process(Policy* policy, Process* process);

/**
 * @brief Selects the next process to be executed according to the policy's rules.
 * @param policy The policy handle.
 * @return The process to execute, or NULL if none are ready.
 */
Process* policy_get_next_process(Policy* policy);

/**
 * @brief Notifies the policy that a clock tick has occurred.
 * @param policy The policy handle.
 */
void policy_tick(Policy* policy);

/**
 * @brief Determines if the scheduler should re-evaluate who is running.
 * 
 * @param policy The policy handle.
 * @param running_process The process currently on the CPU (NULLable)
 * @return true if a scheduling decision should be made, Othewise false if the running_process should continue without interruption
 */
bool policy_needs_reschedule(Policy* policy, Process* running_process);

/**
 * @brief Gets the time quantum for a specific process based on the policy's rules.
 * @param policy The policy handle.
 * @param process The process to query.
 * @return The quantum for the process's current priority level, or 0 if not applicable.
 */
int policy_get_quantum(Policy* policy, Process* process);

/**
 * @brief Informs the policy that a process's quantum has expired.
 *        The policy is responsible for handling the demotion or re-queuing of the process.
 * @param policy The policy handle.
 * @param process The process whose quantum expired.
 */
void policy_demote_process(Policy* policy, Process* process);

#endif // POLICY_INTERFACE_H
