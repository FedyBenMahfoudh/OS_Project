#include "../../headers/engine/policy_interface.h"

// Include headers for ALL concrete policy implementations
// These headers should declare the specific functions like fifo_policy_create, etc.
#include "../../headers/policies/fifo.h"
#include "../../headers/policies/lifo.h"    
#include "../../headers/policies/sjf.h"     
#include "../../headers/policies/priority.h"
#include "../../headers/policies/rr.h"      
#include "../../headers/policies/srt.h"     

#include <stdio.h>  
#include <stdlib.h> 
#include <string.h> 

// --- Internal Policy Wrapper Structure ---
// This structure is internal to policy_interface.c and acts as the "true" Policy
// that gets returned as an opaque `Policy*`.
typedef struct {
    PolicyType type;
    void* concrete_policy_data;
} _Policy;


/**
 * @brief Creates and initializes a new instance of a policy based on its name.
 *        This function acts as a dispatcher, calling the specific create function
 *        for the named policy.
 * @param policy_name The name of the policy to create (e.g., "fifo").
 * @param quantum The time quantum for policies like Round Robin. Ignored otherwise.
 * @return A pointer to the policy's internal state (the handle), or NULL on error.
 */
Policy* policy_create(const char* policy_name, int quantum) {
    _Policy* new_internal_policy = (_Policy*)malloc(sizeof(_Policy));
    if (!new_internal_policy) {
        perror("Policy Interface: Failed to allocate internal _Policy struct");
        return NULL;
    }

    // Dispatch based on policy_name
    if (strcmp(policy_name, "fifo") == 0) {
        new_internal_policy->type = POLICY_TYPE_FIFO;
        new_internal_policy->concrete_policy_data = fifo_policy_create(quantum); // Call FIFO's specific create
    } else if (strcmp(policy_name, "lifo") == 0) {
        new_internal_policy->type = POLICY_TYPE_LIFO;
        new_internal_policy->concrete_policy_data = lifo_policy_create(quantum); // Call LIFO's specific create
    } else if (strcmp(policy_name, "sjf") == 0) {
        new_internal_policy->type = POLICY_TYPE_SJF;
        new_internal_policy->concrete_policy_data = sjf_policy_create(quantum); // Call SJF's specific create
    } else if (strcmp(policy_name, "priority") == 0) {
        new_internal_policy->type = POLICY_TYPE_PRIORITY;
        new_internal_policy->concrete_policy_data = priority_policy_create(quantum); // Call PRIORITY's specific create
    } 
    /*
    else if (strcmp(policy_name, "rr") == 0) { // Commented out until implemented
        new_internal_policy->type = POLICY_TYPE_RR;
        new_internal_policy->concrete_policy_data = rr_policy_create(quantum);
    } else if (strcmp(policy_name, "srt") == 0) { // Commented out until implemented
        new_internal_policy->type = POLICY_TYPE_SRT;
        new_internal_policy->concrete_policy_data = srt_policy_create(quantum);
    }
    */
    else {
        fprintf(stderr, "Policy Interface Error: Policy '%s' not recognized.\n", policy_name);
        free(new_internal_policy); // Policy name not recognized, free wrapper
        return NULL;
    }

    if (!new_internal_policy->concrete_policy_data) {
        fprintf(stderr, "Policy Interface Error: Failed to create concrete policy data for '%s'.\n", policy_name);
        free(new_internal_policy); // Free the generic _Policy wrapper if concrete creation failed
        return NULL;
    }

    return (Policy*)new_internal_policy; // Return as opaque Policy*
}

/**
 * @brief Frees all resources used by the policy.
 *        This function dispatches to the specific destroy function
 *        for the concrete policy type.
 * @param policy The policy handle to destroy.
 */
void policy_destroy(Policy* policy) {
    if (!policy) return;

    _Policy* internal_policy = (_Policy*)policy; // Cast back to internal type

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            fifo_policy_destroy(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_LIFO:
            lifo_policy_destroy(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_SJF:
            sjf_policy_destroy(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_PRIORITY:
            priority_policy_destroy(internal_policy->concrete_policy_data);
            break;
        /*
        case POLICY_TYPE_RR: // Commented out until implemented
            rr_policy_destroy(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_SRT: // Commented out until implemented
            srt_policy_destroy(internal_policy->concrete_policy_data);
            break;
        */
        default:
            fprintf(stderr, "Policy Interface Error: Unknown policy type (%d) in policy_destroy.\n", internal_policy->type);
            break;
    }
    free(internal_policy); // Free the wrapper _Policy struct
}

/**
 * @brief Adds a process to the policy's data structure.
 *        This function dispatches to the specific add_process function.
 * @param policy The policy handle.
 * @param process The process that has become ready.
 */
void policy_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;

    _Policy* internal_policy = (_Policy*)policy; // Cast back to internal type

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            fifo_policy_add_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_LIFO:
            lifo_policy_add_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_SJF:
            sjf_policy_add_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_PRIORITY:
            priority_policy_add_process(internal_policy->concrete_policy_data, process);
            break;
        /*
        case POLICY_TYPE_RR: // Commented out until implemented
            rr_add_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_SRT: // Commented out until implemented
            srt_add_process(internal_policy->concrete_policy_data, process);
            break;
        */
        default:
            fprintf(stderr, "Policy Interface Error: Unknown policy type (%d) in policy_add_process.\n", internal_policy->type);
            break;
    }
}

/**
 * @brief Selects the next process to be executed according to the policy's rules.
 *        This function dispatches to the specific get_next_process function.
 * @param policy The policy handle.
 * @return The process to execute, or NULL if none are ready.
 */
Process* policy_get_next_process(Policy* policy) {
    if (!policy) return NULL;

    _Policy* internal_policy = (_Policy*)policy; // Cast back to internal type

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            return fifo_policy_get_next_process(internal_policy->concrete_policy_data);
        case POLICY_TYPE_LIFO:
            return lifo_policy_get_next_process(internal_policy->concrete_policy_data);
        case POLICY_TYPE_SJF:
            return sjf_policy_get_next_process(internal_policy->concrete_policy_data);
        case POLICY_TYPE_PRIORITY:
            return priority_policy_get_next_process(internal_policy->concrete_policy_data);
        /*
        case POLICY_TYPE_RR: // Commented out until implemented
            return rr_get_next_process(internal_policy->concrete_policy_data);
        case POLICY_TYPE_SRT: // Commented out until implemented
            return srt_get_next_process(internal_policy->concrete_policy_data);
        */
        default:
            fprintf(stderr, "Policy Interface Error: Unknown policy type (%d) in policy_get_next_process.\n", internal_policy->type);
            return NULL;
    }
}

/**
 * @brief Notifies the policy that a clock tick has occurred.
 *        This function dispatches to the specific tick function.
 * @param policy The policy handle.
 */
void policy_tick(Policy* policy) {
    if (!policy) return;

    _Policy* internal_policy = (_Policy*)policy; // Cast back to internal type

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            fifo_policy_tick(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_LIFO:
            lifo_policy_tick(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_SJF:
            sjf_policy_tick(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_PRIORITY:
            priority_policy_tick(internal_policy->concrete_policy_data);
            break;
        /*
        case POLICY_TYPE_RR: // Commented out until implemented
            // if (rr_policy_tick) rr_policy_tick(internal_policy->concrete_policy_data);
            break;
        case POLICY_TYPE_SRT: // Commented out until implemented
            // if (srt_policy_tick) srt_policy_tick(internal_policy->concrete_policy_data);
            break;
        */
        default:
            // This is not an error; many policies don't need a tick function.
            break;
    }
}

/**
 * @brief Determines if the scheduler should re-evaluate who is running.
 *        This function dispatches to the specific needs_reschedule function.
 * @param policy The policy handle.
 * @param running_process The process currently on the CPU.
 * @return true if a scheduling decision should be made.
 */
bool policy_needs_reschedule(Policy* policy, Process* running_process) {
    if (!policy) return true; // Fail safe: if no policy, assume reschedule.
    
    _Policy* internal_policy = (_Policy*)policy;

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            return fifo_policy_needs_reschedule(internal_policy->concrete_policy_data, running_process);
        case POLICY_TYPE_LIFO:
            return lifo_policy_needs_reschedule(internal_policy->concrete_policy_data, running_process);
        case POLICY_TYPE_SJF:
            return sjf_policy_needs_reschedule(internal_policy->concrete_policy_data, running_process);
        case POLICY_TYPE_PRIORITY:
            return priority_policy_needs_reschedule(internal_policy->concrete_policy_data, running_process);
        /*
        case POLICY_TYPE_SRT: // Commented out until implemented
            return srt_needs_reschedule(internal_policy->concrete_policy_data, running_process);
        */
        default:
            fprintf(stderr, "Policy Interface Error: Unknown policy type (%d) in policy_needs_reschedule.\n", internal_policy->type);
            return true;
    }
}

int policy_get_quantum(Policy* policy, Process* process) {
    if (!policy) return 0;
    _Policy* internal_policy = (_Policy*)policy;

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            return fifo_policy_get_quantum(internal_policy->concrete_policy_data, process);
        case POLICY_TYPE_LIFO:
            return lifo_policy_get_quantum(internal_policy->concrete_policy_data, process);
        case POLICY_TYPE_SJF:
            return sjf_policy_get_quantum(internal_policy->concrete_policy_data, process);
        case POLICY_TYPE_PRIORITY:
            return priority_policy_get_quantum(internal_policy->concrete_policy_data, process);
        // Add cases for RR, SRT, MLFQ here later
        default:
            return 0; // Default to no quantum
    }
}

void policy_demote_process(Policy* policy, Process* process) {
    if (!policy) return;
    _Policy* internal_policy = (_Policy*)policy;

    switch (internal_policy->type) {
        case POLICY_TYPE_FIFO:
            fifo_policy_demote_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_LIFO:
            lifo_policy_demote_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_SJF:
            sjf_policy_demote_process(internal_policy->concrete_policy_data, process);
            break;
        case POLICY_TYPE_PRIORITY:
            priority_policy_demote_process(internal_policy->concrete_policy_data, process);
            break;
        // Add cases for RR, SRT, MLFQ here later
        default:
            // Default to no-op
            break;
    }
}
