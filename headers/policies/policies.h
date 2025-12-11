#ifndef POLICIES_H
#define POLICIES_H

#include <stdbool.h>
#include "../data_structures/process.h"

// Forward declaration of the opaque Policy handle
typedef struct Policy Policy;

// The VTable structure for a policy, containing all function pointers.
typedef struct PolicyVTable {
    const char* name;
    void* (*create)(int quantum);
    void (*destroy)(void* policy_data);
    void (*add_process)(void* policy_data, Process* process);
    Process* (*get_next_process)(void* policy_data);
    void (*tick)(void* policy_data);
    bool (*needs_reschedule)(void* policy_data, Process* running_process);
    int (*get_quantum)(void* policy_data, Process* process);
    void (*demote_process)(void* policy_data, Process* process);
} PolicyVTable;

/**
 * @brief Registers a policy's vtable with the central registrar.
 * @param vtable A pointer to the policy's static vtable.
 */
void register_policy(const PolicyVTable* vtable);

/**
 * @brief Retrieves the names of all currently registered policies.
 * @param count A pointer to an integer that will be filled with the number of policies.
 * @return An array of strings containing the policy names.
 */
const char** get_available_policies(int* count);

/**
 * @brief Calls the registration function for every available policy.
 *        This function must be implemented to call register_policy for each policy's vtable.
 */
void register_all_policies();

#endif // POLICIES_H
