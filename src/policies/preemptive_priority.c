#include "../../headers/policies/preemptive_priority.h"
#include "../../headers/data_structures/max_heap.h"
#include <stdlib.h>

// --- Internal Preemptive Priority Policy Data Structure ---
typedef struct {
    MaxHeap* heap;
} PreemptivePriorityPolicyData;

// --- Comparator for the MaxHeap ---
static int preemptive_priority_comparator(Process* a, Process* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    // Primary Key: Priority (Higher is better)
    if (p1->priority != p2->priority) {
        return p1->priority - p2->priority;
    }

    // Secondary Key: Last Executed Time (Smaller is better -> ran longer ago or never ran)
    // We want 'a' to be > 'b' (higher preference) if a.last_exec < b.last_exec
    if (p1->last_executed_time < p2->last_executed_time) return 1;
    if (p1->last_executed_time > p2->last_executed_time) return -1;

    // Tertiary Key: Arrival Time (Smaller is better)
    return p2->arrival_time - p1->arrival_time;
}

// --- Static (Private) Policy Functions ---

static void* preemptive_priority_create(int quantum) {
    // Ignoring the quantum
    (void)quantum;  
    PreemptivePriorityPolicyData* policy_data = (PreemptivePriorityPolicyData*)malloc(sizeof(PreemptivePriorityPolicyData));
    if (!policy_data) return NULL;

    policy_data->heap = max_heap_create(preemptive_priority_comparator);
    if (!policy_data->heap) {
        free(policy_data);
        return NULL;
    }
    return policy_data;
}

static void preemptive_priority_destroy(void* policy_data) {
    if (!policy_data) return;
    PreemptivePriorityPolicyData* data = (PreemptivePriorityPolicyData*)policy_data;
    max_heap_destroy(data->heap);
    free(data);
}

static void preemptive_priority_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    PreemptivePriorityPolicyData* data = (PreemptivePriorityPolicyData*)policy_data;
    max_heap_push(data->heap, process);
}

static Process* preemptive_priority_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    PreemptivePriorityPolicyData* data = (PreemptivePriorityPolicyData*)policy_data;
    if (max_heap_is_empty(data->heap)) return NULL;
    return max_heap_pop(data->heap);
}

static void preemptive_priority_tick(void* policy_data) {
    (void)policy_data;
}

static bool preemptive_priority_needs_reschedule(void* policy_data, Process* running_process) {
    // 1. If no process is running, we must reschedule
    if (running_process == NULL) return true;

    PreemptivePriorityPolicyData* data = (PreemptivePriorityPolicyData*)policy_data;
    
    // 2. Peek at the highest priority process in the queue
    if (max_heap_is_empty(data->heap)) return false;
    
    Process* best_waiting = max_heap_peek(data->heap);
    
    // 3. Preempt if the waiting process has strictly higher priority
    if (best_waiting && best_waiting->priority > running_process->priority) {
        return true;
    }

    return false;
}

static int preemptive_priority_get_quantum(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
    return 0;
}

static void preemptive_priority_demote_process(void* policy_data, Process* process) {
    if (process) {
        preemptive_priority_add_process(policy_data, process);
    }
}

// --- VTable Definition ---

static const PolicyVTable preemptive_priority_vtable = {
    .name = "preemptive_priority",
    .create = preemptive_priority_create,
    .destroy = preemptive_priority_destroy,
    .add_process = preemptive_priority_add_process,
    .get_next_process = preemptive_priority_get_next_process,
    .tick = preemptive_priority_tick,
    .needs_reschedule = preemptive_priority_needs_reschedule,
    .get_quantum = preemptive_priority_get_quantum,
    .demote_process = preemptive_priority_demote_process
};

// --- Public VTable Accessor ---

const PolicyVTable* preemptive_priority_get_vtable() {
    return &preemptive_priority_vtable;
}
