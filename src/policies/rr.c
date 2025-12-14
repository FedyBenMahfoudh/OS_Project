#include "../../headers/policies/rr.h"
#include "../../headers/data_structures/queue.h"
#include <stdlib.h>

typedef struct {
    Queue* ready_queue;
    int quantum;
} RrPolicyData;

static void* rr_create(int quantum) {
    RrPolicyData* policy_data = (RrPolicyData*)malloc(sizeof(RrPolicyData));
    if (!policy_data) return NULL;

    policy_data->ready_queue = queue_create();
    if (!policy_data->ready_queue) {
        free(policy_data);
        return NULL;
    }
    policy_data->quantum = (quantum > 0) ? quantum : 1;
    return policy_data;
}

static void rr_destroy(void* policy_data) {
    if (!policy_data) return;
    RrPolicyData* rr_data = (RrPolicyData*)policy_data;
    queue_destroy(rr_data->ready_queue);
    free(rr_data);
}

static void rr_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    RrPolicyData* rr_data = (RrPolicyData*)policy_data;
    queue_enqueue(rr_data->ready_queue, process);
}

static Process* rr_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    RrPolicyData* rr_data = (RrPolicyData*)policy_data;
    if (queue_is_empty(rr_data->ready_queue)) return NULL;
    return queue_dequeue(rr_data->ready_queue);
}

static void rr_tick(void* policy_data) {
    (void)policy_data;
}

static bool rr_needs_reschedule(void* policy_data, Process* running_process) {
    if (!policy_data) return true;
    RrPolicyData* rr_data = (RrPolicyData*)policy_data;
    
    return running_process == NULL || (running_process->current_quantum_runtime >= rr_data->quantum);
}

static int rr_get_quantum(void* policy_data, Process* process) {
    (void)process;
    if (!policy_data) return 0;
    RrPolicyData* rr_data = (RrPolicyData*)policy_data;
    return rr_data->quantum;
}

static void rr_demote_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    RrPolicyData* rr_data = (RrPolicyData*)policy_data;
    
    process->current_quantum_runtime = 0;
    
    rr_add_process(rr_data, process);
}

static const PolicyVTable rr_vtable = {
    .name = "rr",
    .create = rr_create,
    .destroy = rr_destroy,
    .add_process = rr_add_process,
    .get_next_process = rr_get_next_process,
    .tick = rr_tick,
    .needs_reschedule = rr_needs_reschedule,
    .get_quantum = rr_get_quantum,
    .demote_process = rr_demote_process
};

const PolicyVTable* rr_get_vtable() {
    return &rr_vtable;
}