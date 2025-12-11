#include "../../headers/policies/sjf.h"
#include "../../headers/data_structures/min_heap.h"
#include <stdlib.h>

// --- Internal SJF Policy Data Structure ---
typedef struct {
    MinHeap* heap;
} SjfPolicyData;

// --- Comparator for the MinHeap ---
static int sjf_comparator(void* a, void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    return p1->burst_time - p2->burst_time;
}

// --- Static (Private) Policy Functions ---

static void* sjf_create(int quantum) {
    // Ignoring the quantum
    (void)quantum;  
    SjfPolicyData* policy_data = (SjfPolicyData*)malloc(sizeof(SjfPolicyData));
    if (!policy_data) return NULL;

    policy_data->heap = min_heap_create(sjf_comparator);
    if (!policy_data->heap) {
        free(policy_data);
        return NULL;
    }
    return policy_data;
}

static void sjf_destroy(void* policy_data) {
    if (!policy_data) return;
    SjfPolicyData* sjf_data = (SjfPolicyData*)policy_data;
    min_heap_destroy(sjf_data->heap);
    free(sjf_data);
}

static void sjf_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    SjfPolicyData* sjf_data = (SjfPolicyData*)policy_data;
    min_heap_push(sjf_data->heap, process);
}

static Process* sjf_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    SjfPolicyData* sjf_data = (SjfPolicyData*)policy_data;
    if (min_heap_is_empty(sjf_data->heap)) return NULL;
    return min_heap_pop(sjf_data->heap);
}

static void sjf_tick(void* policy_data) {
    (void)policy_data;
}

static bool sjf_needs_reschedule(void* policy_data, Process* running_process) {
    (void)policy_data;
    // When the CPU is idle.
    return running_process == NULL;
}

static int sjf_get_quantum(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
    return 0;
}

static void sjf_demote_process(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
}

// --- VTable Definition ---

static const PolicyVTable sjf_vtable = {
    .name = "sjf",
    .create = sjf_create,
    .destroy = sjf_destroy,
    .add_process = sjf_add_process,
    .get_next_process = sjf_get_next_process,
    .tick = sjf_tick,
    .needs_reschedule = sjf_needs_reschedule,
    .get_quantum = sjf_get_quantum,
    .demote_process = sjf_demote_process
};

// --- Public VTable Accessor ---

const PolicyVTable* sjf_get_vtable() {
    return &sjf_vtable;
}
