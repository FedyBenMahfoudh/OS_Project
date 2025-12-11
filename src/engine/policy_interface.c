/**
 * @file policy_interface.c
 * @brief Implements the Policy Interface for managing and interacting with various scheduling policies.
 *
 * This file provides a generic interface allowing the scheduler engine to interact
 * with different CPU scheduling policies (e.g., FIFO, RR, Priority) through a
 * virtual table (VTable) mechanism. It handles policy registration, creation,
 * destruction, and all common operations like adding processes, getting the next
 * process, and handling time ticks and rescheduling events.
 */

#include "../../headers/engine/policy_interface.h"
#include "../../headers/policies/policies.h" // The new header with VTable info
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Maximum number of policies that can be registered.
 */
#define MAX_POLICIES 10

/**
 * @brief Static array to hold pointers to all registered PolicyVTables.
 */
static const PolicyVTable* policy_registrar[MAX_POLICIES];

/**
 * @brief Counter for the number of currently registered policies.
 */
static int registered_policy_count = 0;

/**
 * @brief Cache for storing names of registered policies.
 */
static const char* policy_names[MAX_POLICIES]; // Cache for policy names

/**
 * @brief Internal representation of a Policy.
 * This structure holds the VTable for policy-specific operations
 * and a pointer to the concrete policy data. It is opaque to external modules.
 */
struct Policy {
    const PolicyVTable* vtable; /**< Pointer to the virtual table for policy operations. */
    void* concrete_policy_data; /**< Pointer to the actual policy-specific data structure. */
};

/**
 * @brief Registers a scheduling policy with the system.
 *
 * This function adds a given PolicyVTable to the internal registrar, making
 * the policy available for use by the scheduler engine.
 *
 * @param vtable A pointer to the constant PolicyVTable structure of the policy to register.
 */
void register_policy(const PolicyVTable* vtable) {
    if (registered_policy_count < MAX_POLICIES) {
        policy_registrar[registered_policy_count] = vtable;
        policy_names[registered_policy_count] = vtable->name;
        registered_policy_count++;
    } else {
        fprintf(stderr, "Policy Registrar Error: Exceeded maximum number of policies (%d).\n", MAX_POLICIES);
    }
}

/**
 * @brief Retrieves an array of names of all available (registered) policies.
 *
 * @param count A pointer to an integer where the number of available policies will be stored.
 * @return A constant array of strings, where each string is the name of a registered policy.
 */
const char** get_available_policies(int* count) {
    *count = registered_policy_count;
    return (const char**)policy_names;
}

/**
 * @brief Creates a new instance of a specified scheduling policy.
 *
 * This function looks up the policy by name and, if found, uses its VTable's
 * create function to instantiate the policy-specific data and wrap it in a
 * generic Policy structure.
 *
 * @param policy_name The name of the policy to create (e.g., "fifo", "rr").
 * @param quantum The time quantum to use for quantum-based policies (ignored by others).
 * @return A pointer to a newly created Policy object, or NULL if creation fails or policy is not found.
 */
Policy* policy_create(const char* policy_name, int quantum) {
    const PolicyVTable* vtable = NULL;
    for (int i = 0; i < registered_policy_count; i++) {
        if (strcmp(policy_registrar[i]->name, policy_name) == 0) {
            vtable = policy_registrar[i];
            break;
        }
    }

    if (!vtable) {
        fprintf(stderr, "Policy Interface Error: Policy '%s' not recognized or not registered.\n", policy_name);
        return NULL;
    }

    Policy* new_internal_policy = (Policy*)malloc(sizeof(Policy));
    if (!new_internal_policy) {
        perror("Policy Interface: Failed to allocate internal Policy struct");
        return NULL;
    }

    new_internal_policy->vtable = vtable;
    new_internal_policy->concrete_policy_data = vtable->create(quantum);

    if (!new_internal_policy->concrete_policy_data) {
        fprintf(stderr, "Policy Interface Error: Failed to create concrete policy data for '%s'.\n", policy_name);
        free(new_internal_policy);
        return NULL;
    }

    return new_internal_policy;
}

/**
 * @brief Destroys a Policy instance and frees its associated resources.
 *
 * This function calls the policy-specific destroy function from its VTable
 * to clean up the concrete policy data, and then frees the generic Policy
 * structure itself.
 *
 * @param policy A pointer to the Policy object to destroy.
 */
void policy_destroy(Policy* policy) {
    if (!policy) return;
    policy->vtable->destroy(policy->concrete_policy_data);
    free(policy);
}

/**
 * @brief Adds a process to the scheduling policy's ready queue.
 *
 * This function delegates the process addition to the policy-specific
 * add_process function.
 *
 * @param policy A pointer to the Policy object.
 * @param process A pointer to the Process to add.
 */
void policy_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;
    policy->vtable->add_process(policy->concrete_policy_data, process);
}

/**
 * @brief Retrieves the next process to be run by the CPU according to the policy.
 *
 * This function delegates the selection of the next process to the policy-specific
 * get_next_process function. The returned process is typically removed from the
 * policy's internal ready queue.
 *
 * @param policy A pointer to the Policy object.
 * @return A pointer to the next Process to run, or NULL if no processes are ready.
 */
Process* policy_get_next_process(Policy* policy) {
    if (!policy) return NULL;
    return policy->vtable->get_next_process(policy->concrete_policy_data);
}

/**
 * @brief Notifies the policy that a time tick has occurred.
 *
 * This function allows the policy to perform any per-tick operations, such as
 * aging, updating internal timers, or checking for preemption conditions.
 * It's an optional callback; policies that don't need it can leave it NULL.
 *
 * @param policy A pointer to the Policy object.
 */
void policy_tick(Policy* policy) {
    if (!policy) return;
    // Tick is optional
    if (policy->vtable->tick) {
        policy->vtable->tick(policy->concrete_policy_data);
    }
}

/**
 * @brief Determines if the scheduler needs to reconsider which process should run.
 *
 * This function delegates to the policy-specific needs_reschedule function,
 * which can indicate preemption due to a higher priority process, quantum expiry,
 * or other policy-specific conditions.
 *
 * @param policy A pointer to the Policy object.
 * @param running_process A pointer to the currently running process, or NULL if CPU is idle.
 * @return true if a reschedule is needed, false otherwise.
 */
bool policy_needs_reschedule(Policy* policy, Process* running_process) {
    if (!policy) return true;
    if (policy->vtable->needs_reschedule) {
        return policy->vtable->needs_reschedule(policy->concrete_policy_data, running_process);
    }
    return true; // Default to true if not implemented, ensuring active scheduling
}

/**
 * @brief Retrieves the time quantum assigned to a specific process by the policy.
 *
 * This function delegates to the policy-specific get_quantum function. It's
 * primarily used by quantum-based policies like Round Robin or MLFQ.
 *
 * @param policy A pointer to the Policy object.
 * @param process A pointer to the Process for which to get the quantum.
 * @return The time quantum for the process, or 0 if the policy doesn't use quanta
 *         or the process doesn't have a specific quantum.
 */
int policy_get_quantum(Policy* policy, Process* process) {
    if (!policy) return 0;
    if (policy->vtable->get_quantum) {
        return policy->vtable->get_quantum(policy->concrete_policy_data, process);
    }
    return 0; // Policies that don't use quantum will return 0
}

/**
 * @brief Notifies the policy to potentially demote a process (e.g., due to quantum expiry).
 *
 * This function is used by policies like MLFQ to move processes to lower priority
 * queues after they have consumed their time slice or allotment.
 * It's an optional callback; policies that don't need it can leave it NULL.
 *
 * @param policy A pointer to the Policy object.
 * @param process A pointer to the Process to potentially demote.
 */
void policy_demote_process(Policy* policy, Process* process) {
    if (!policy) return;
    if (policy->vtable->demote_process) {
        policy->vtable->demote_process(policy->concrete_policy_data, process);
    }
}