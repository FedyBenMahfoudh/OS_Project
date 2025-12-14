#include "../../headers/policies/srt.h"
#include "../../headers/data_structures/min_heap.h"
#include <stdlib.h>

typedef struct {
    MinHeap* ready_queue;
} SrtPolicyData;

static int srt_comparator(Process* a, Process* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;

    if (p1->remaining_burst_time != p2->remaining_burst_time) {
        return p1->remaining_burst_time - p2->remaining_burst_time;
    }

    if (p1->last_executed_time != p2->last_executed_time) {
        return p1->last_executed_time - p2->last_executed_time;
    }

    return p1->arrival_time - p2->arrival_time;
}

static void* srt_create(int quantum) {
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
    (void)policy_data;
}

static bool srt_needs_reschedule(void* policy_data, Process* running_process) {
    if (!policy_data) return true;
    SrtPolicyData* srt_data = (SrtPolicyData*)policy_data;

    if (running_process == NULL) {
        return true;
    }
    

    if (running_process->state == TERMINATED) {
        return true;
    }

    Process* shortest_in_queue = min_heap_peek(srt_data->ready_queue);
    if (shortest_in_queue && shortest_in_queue->remaining_burst_time < running_process->remaining_burst_time) {
        return true;
    }

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

const PolicyVTable* srt_get_vtable() {
    return &srt_vtable;
}
