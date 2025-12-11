#include "../../headers/policies/srt.h"
#include "../../headers/data_structures/min_heap.h"
#include <stdlib.h>

// --- Internal SRT Policy Data Structure ---
typedef struct {
    MinHeap* ready_queue;
} SrtPolicyData;

// --- Comparator for the MinHeap ---
static int srt_comparator(Process* a, Process* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    // Primary: Shortest Remaining Time First
    if (p1->remaining_burst_time != p2->remaining_burst_time) {
        return p1->remaining_burst_time - p2->remaining_burst_time;
    }

    // Secondary: Prefer processes that haven't run recently (or at all)
    // Smaller last_executed_time comes first
    if (p1->last_executed_time != p2->last_executed_time) {
        return p1->last_executed_time - p2->last_executed_time;
    }

    // Tertiary: First Come First Served
    return p1->arrival_time - p2->arrival_time;
}

// --- Static (Private) Policy Functions ---

static void* srt_create(int quantum) {
    // Ignoring the quantum
    (void)quantum; 
    SrtPolicyData* policy_data = (SrtPolicyData*)malloc(sizeof(SrtPolicyData));
    if (!policy_data) return NULL;

    policy_data->ready_queue = min_heap_create(srt_comparator);
    if (!policy_data->ready_queue) {
        free(policy_data);
        return NULL;
    }
    return policy_data;
}

static void srt_destroy(void* policy_data) {
    if (!policy_data) return;
    SrtPolicyData* srt_data = (SrtPolicyData*)policy_data;
    min_heap_destroy(srt_data->ready_queue);
    free(srt_data);
}

static void srt_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    SrtPolicyData* srt_data = (SrtPolicyData*)policy_data;
    min_heap_push(srt_data->ready_queue, process);
}

static Process* srt_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    SrtPolicyData* srt_data = (SrtPolicyData*)policy_data;
    if (min_heap_is_empty(srt_data->ready_queue)) return NULL;
    return min_heap_pop(srt_data->ready_queue);
}

static void srt_tick(void* policy_data) {
    // The logic of decrementing burst time should be in the scheduler engine,
    // not in the policy's tick function. A policy should only manage its data structures.
    // So, this is intentionally a no-op.
    (void)policy_data;
}

static bool srt_needs_reschedule(void* policy_data, Process* running_process) {
    if (!policy_data) return true;
    SrtPolicyData* srt_data = (SrtPolicyData*)policy_data;

    // Scheduling a new process if the CPU is idle.
    if (running_process == NULL) {
        return true;
    }
    
    // Scheduling a new process if the running process has terminated.
    if (running_process->state == TERMINATED) {
        return true;
    }

    // Scheduling a new process if there's a process in the queue that has a shorter remaining time
    // than the currently running process.
    Process* shortest_in_queue = min_heap_peek(srt_data->ready_queue);
    if (shortest_in_queue && shortest_in_queue->remaining_burst_time < running_process->remaining_burst_time) {
        return true;
    }

    // Otherwise, letting the current process continue.
    return false;
}

static int srt_get_quantum(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
    return 0;
}

static void srt_demote_process(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
}

// --- VTable Definition ---

static const PolicyVTable srt_vtable = {
    .name = "srt",
    .create = srt_create,
    .destroy = srt_destroy,
    .add_process = srt_add_process,
    .get_next_process = srt_get_next_process,
    .tick = srt_tick,
    .needs_reschedule = srt_needs_reschedule,
    .get_quantum = srt_get_quantum,
    .demote_process = srt_demote_process
};

// --- Public VTable Accessor ---

const PolicyVTable* srt_get_vtable() {
    return &srt_vtable;
}
