#include "../../headers/policies/preemptive_priority.h"
#include "../../headers/data_structures/max_heap.h"
#include <stdlib.h>

typedef struct {
    MaxHeap* heap;
} PreemptivePriorityPolicyData;

static int preemptive_priority_comparator(Process* a, Process* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    if (p1->priority != p2->priority) {
        return p1->priority - p2->priority;
    }

    if (p1->last_executed_time < p2->last_executed_time) return 1;
    if (p1->last_executed_time > p2->last_executed_time) return -1;

    return p2->arrival_time - p1->arrival_time;
}

static void* preemptive_priority_create(int quantum) {
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

    if (running_process == NULL) return true;

    PreemptivePriorityPolicyData* data = (PreemptivePriorityPolicyData*)policy_data;
    

    if (max_heap_is_empty(data->heap)) return false;
    
    Process* best_waiting = max_heap_peek(data->heap);
    

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


const PolicyVTable* preemptive_priority_get_vtable() {
    return &preemptive_priority_vtable;
}
