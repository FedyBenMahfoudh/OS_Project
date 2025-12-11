#include "../../headers/policies/priority.h"
#include "../../headers/data_structures/max_heap.h"
#include <stdlib.h>

// --- Internal Priority Policy Data Structure ---
typedef struct {
    MaxHeap* heap;
} PriorityPolicyData;

// --- Comparator for the MaxHeap ---
static int priority_comparator(Process* a, Process* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    // Higher priority value is greater
    if (p1->priority != p2->priority) {
        return p1->priority - p2->priority;
    }
    // Tie-break with arrival time: earlier arrival is greater
    return p2->arrival_time - p1->arrival_time;
}

// --- Static (Private) Policy Functions ---

static void* priority_create(int quantum) {
    // Ignoring the quantum
    (void)quantum; 
    PriorityPolicyData* policy_data = (PriorityPolicyData*)malloc(sizeof(PriorityPolicyData));
    if (!policy_data) return NULL;

    policy_data->heap = max_heap_create(priority_comparator);
    if (!policy_data->heap) {
        free(policy_data);
        return NULL;
    }
    return policy_data;
}

static void priority_destroy(void* policy_data) {
    if (!policy_data) return;
    PriorityPolicyData* priority_data = (PriorityPolicyData*)policy_data;
    max_heap_destroy(priority_data->heap);
    free(priority_data);
}

static void priority_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    PriorityPolicyData* priority_data = (PriorityPolicyData*)policy_data;
    max_heap_push(priority_data->heap, process);
}

static Process* priority_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    PriorityPolicyData* priority_data = (PriorityPolicyData*)policy_data;
    if (max_heap_is_empty(priority_data->heap)) return NULL;
    return max_heap_pop(priority_data->heap);
}

static void priority_tick(void* policy_data) {
    (void)policy_data;
}

static bool priority_needs_reschedule(void* policy_data, Process* running_process) {
    (void)policy_data;
    // When the CPU is idle.
    return running_process == NULL;
}

static int priority_get_quantum(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
    return 0; 
}

static void priority_demote_process(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process; 
}

// --- VTable Definition ---

static const PolicyVTable priority_vtable = {
    .name = "priority",
    .create = priority_create,
    .destroy = priority_destroy,
    .add_process = priority_add_process,
    .get_next_process = priority_get_next_process,
    .tick = priority_tick,
    .needs_reschedule = priority_needs_reschedule,
    .get_quantum = priority_get_quantum,
    .demote_process = priority_demote_process
};

// --- Public VTable Accessor ---

const PolicyVTable* priority_get_vtable() {
    return &priority_vtable;
}
