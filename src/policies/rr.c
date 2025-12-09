/**
 * @file rr.c
 * @brief Implementation of the Round Robin (RR) scheduling policy.
 *
 * The Round Robin policy is a preemptive scheduling algorithm that
 * cycles through processes in a FIFO order, giving each process a fixed
 * time slice (quantum). In this module, we focus on the data structure
 * and policy-specific state: a ready queue and the quantum value.
 *
 * The actual preemption logic (i.e., deciding when a quantum expires
 * and when to reinsert a running process at the end of the queue) is
 * the responsibility of the scheduler engine. The engine will:
 *   - Call rr_add_process() when a process becomes READY or when a
 *     running process is preempted and must go back to the READY queue.
 *   - Call rr_get_next_process() whenever it needs to select the next
 *     process to run.
 */

#include "../../headers/policies/rr.h"
#include "../../headers/data_structures/data_structures.h"

#include <stdlib.h>

/**
 * @struct Policy
 * @brief Internal state for the Round Robin scheduling policy.
 *
 * This structure is opaque outside of this module. It stores:
 *  - A FIFO queue of READY processes.
 *  - The configured time quantum.
 */
struct Policy {
    Queue* ready_queue; /**< FIFO queue of ready processes. */
    int    quantum;     /**< Time quantum used by the RR policy. */
};

/**
 * @brief Creates and initializes a new Round Robin (RR) policy instance.
 *
 * The Round Robin policy uses a FIFO queue internally to store READY
 * processes and a fixed time quantum to determine the time slice for
 * each process. The quantum value is provided by the caller and stored
 * inside the policy handle so that the simulation engine can use it
 * when applying preemptive scheduling.
 *
 * @param quantum The time quantum (must be > 0). If a non-positive value
 *                is provided, a default quantum of 1 will be used.
 * @return Pointer to the created Policy instance, or NULL on failure.
 */
Policy* rr_policy_create(int quantum) {
    Policy* rr_policy = (Policy*) malloc(sizeof(Policy));
    if (!rr_policy) {
        return NULL;
    }

    rr_policy->ready_queue = queue_create();
    if (!rr_policy->ready_queue) {
        free(rr_policy);
        return NULL;
    }

    // Ensure a strictly positive quantum; default to 1 if invalid.
    if (quantum <= 0) {
        quantum = 1;
    }
    rr_policy->quantum = quantum;

    return rr_policy;
}


/**
 * @brief Destroys a Round Robin policy instance and frees its resources.
 *
 * @param policy Pointer to the Policy instance to destroy (can be NULL).
 */
void rr_policy_destroy(Policy* policy) {
    if (!policy) {
        return;
    }

    if (policy->ready_queue) {
        queue_destroy(policy->ready_queue);
    }

    free(policy);
}

/**
 * @brief Adds a process to the Round Robin ready queue.
 *
 * The process is appended to the end of the internal FIFO queue. The
 * engine should call this function whenever a process becomes READY or
 * when a running process is preempted and must be reinserted at the tail
 * of the queue.
 *
 * @param policy  Pointer to the RR policy instance.
 * @param process Pointer to the Process to enqueue.
 */
void rr_policy_add_process(Policy* policy, Process* process) {
    if (!policy || !process) {
        return;
    }

    queue_enqueue(policy->ready_queue, process);
}


/**
 * @brief Retrieves the next process to execute according to RR order.
 *
 * This function removes and returns the process at the front of the
 * internal queue. If the queue is empty or policy is NULL, it returns
 * NULL.
 *
 * @param policy Pointer to the RR policy instance.
 * @return Pointer to the next Process to execute, or NULL if none.
 */
Process* rr_policy_get_next_process(Policy* policy) {
    if (!policy || queue_is_empty(policy->ready_queue)) {
        return NULL;
    }

    return queue_dequeue(policy->ready_queue);
}


int rr_policy_get_quantum(Policy* policy, Process* process) {
    return policy->quantum;
}

void rr_policy_tick(Policy* policy) {
    // This Round Robin policy is non-preemptive and doesn't use aging,
    // so the tick function is a no-op.
    (void)policy;
}

bool rr_policy_needs_reschedule(Policy* policy, Process* running_process) {
    // Rescheduling only when the process is idle or Quantum expired
    return (running_process == NULL || running_process->current_quantum_runtime >= policy->quantum);
}

void rr_policy_demote_process(Policy* policy, Process* process){
    process->current_quantum_runtime = 0;
    rr_policy_add_process(policy, process);
}


