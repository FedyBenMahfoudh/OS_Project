#include "../../headers/policies/srt.h"
#include "../../headers/data_structures/data_structures.h"
#include <stdlib.h>

struct Policy {
    MinHeap* ready_queue;
    Process* current_process;
};

/**
 * @brief Comparator function for SRT (Shortest Remaining Time).
 * @param a First process.
 * @param b Second process.
 * @return <0 if a < b, 0 if a == b, >0 if a > b (based on remaining_burst_time).
 */
static int srt_compare_processes(Process* a, Process* b) {
    return a->remaining_burst_time - b->remaining_burst_time;
}

/**
 * Creates a new SRT policy.
 * @param quantum Time quantum (ignored for SRT).
 * @return A pointer to the new Policy.
 */
Policy* srt_policy_create(int quantum) {
    (void)quantum; 

    Policy* srt_policy = (Policy*)malloc(sizeof(Policy));
    if (!srt_policy) return NULL;

    srt_policy->ready_queue = min_heap_create(srt_compare_processes);
    if (!srt_policy->ready_queue) {
        free(srt_policy);
        return NULL;
    }

    srt_policy->current_process = NULL;
    return srt_policy;
}

/**
 * Destroys the SRT policy and frees memory.
 * @param policy The policy to destroy.
 */
void srt_policy_destroy(Policy* policy) {
    if (!policy) return;

    min_heap_destroy(policy->ready_queue);
    free(policy);
}

/**
 * Adds a process to the SRT policy.
 * @param policy The policy.
 * @param process The process to add.
 */
void srt_policy_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;

    process->state = READY;
    min_heap_push(policy->ready_queue, process);
}

/**
 * Gets the next process to run from the SRT policy.
 * @param policy The policy.
 * @return The next process to run, or NULL if none.
 */
Process* srt_policy_get_next_process(Policy* policy) {
    if (!policy) return NULL;

    if (policy->current_process && 
        policy->current_process->state != TERMINATED &&
        policy->current_process->remaining_burst_time > 0) {
        min_heap_push(policy->ready_queue, policy->current_process);
    }

    policy->current_process = min_heap_pop(policy->ready_queue);
    if (policy->current_process) {
        policy->current_process->state = RUNNING;
    }
    
    return policy->current_process;
}

/**
 * Called on each timer tick.
 * @param policy The policy.
 */
void srt_policy_tick(Policy* policy) {
    if (!policy || !policy->current_process) return;
    
    if (policy->current_process->remaining_burst_time > 0) {
        policy->current_process->remaining_burst_time--;
        policy->current_process->executed_time++;
        
        if (policy->current_process->remaining_burst_time <= 0) {
            policy->current_process->state = TERMINATED;
            //policy->current_process->finish_time = ?? idk hhh
        }
    }
}

/**
 * Determines if a reschedule is needed.
 * @param policy The policy.
 * @param running_process The currently running process.
 * @return True if a reschedule is needed, false otherwise.
 */
bool srt_policy_needs_reschedule(Policy* policy, Process* running_process) {
    if (!policy) return false;
    
    if (!running_process) return true;
    
    if (running_process->state == TERMINATED) return true;
    
    Process* shortest = min_heap_peek(policy->ready_queue);
    if (shortest && shortest->remaining_burst_time < running_process->remaining_burst_time) {
        return true;
    }
    
    return false;
}

/**
 * Gets the time quantum for a specific process.
 * @param policy The policy.
 * @param process The process.
 * @return The time quantum for the process.
 */
int srt_policy_get_quantum(Policy* policy, Process* process) {
    (void)policy;
    (void)process;
    return 0; // Not applicable
}

/**
 * Handles process demotion (quantum expiry).
 * For SRT, we don't use quantum-based preemption, so this is a no-op.
 */
void srt_policy_demote_process(Policy* policy, Process* process) {
    // No-op for SRT
    (void)policy;
    (void)process;
}