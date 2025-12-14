#include "../../headers/policies/mlfq.h"
#include "../../headers/data_structures/queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_PRIORITY_LEVELS 20
#define AGING_THRESHOLD 10  
#define TIME_ALLOTMENT_RATIO 5 

typedef struct {
    Queue* queues[MAX_PRIORITY_LEVELS];
    int base_quantum;
    int current_time;
} MlfqPolicyData;

static void* mlfq_create(int quantum) {
    MlfqPolicyData* data = (MlfqPolicyData*)malloc(sizeof(MlfqPolicyData));
    if (!data) return NULL;

    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++) {
        data->queues[i] = queue_create();
        if (!data->queues[i]) {
            for (int j = 0; j < i; j++) queue_destroy(data->queues[j]);
            free(data);
            return NULL;
        }
    }
    
    data->base_quantum = (quantum > 0) ? quantum : 1;
    data->current_time = 0;
    return data;
}

static void mlfq_destroy(void* policy_data) {
    if (!policy_data) return;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;
    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++) {
        queue_destroy(data->queues[i]);
    }
    free(data);
}

static void mlfq_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;
    
    int level = process->priority;
    if (level >= MAX_PRIORITY_LEVELS) level = MAX_PRIORITY_LEVELS - 1;
    if (level < 0) level = 0;
    
    process->current_queue_level = level;
    process->current_quantum_runtime = 0;
    process->time_spent_at_current_level = 0;
    process->last_active_time = data->current_time;
    
    queue_enqueue(data->queues[level], process);
}

static Process* mlfq_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;
 
    for (int i = MAX_PRIORITY_LEVELS - 1; i >= 0; i--) {
        if (!queue_is_empty(data->queues[i])) {
            Process* p = queue_dequeue(data->queues[i]);
            return p;
        }
    }
    return NULL;
}

static int get_quantum_for_level(MlfqPolicyData* data, int level) {
    if (level >= 15) return data->base_quantum * 1;
    if (level >= 10) return data->base_quantum * 2;
    if (level >= 5)  return data->base_quantum * 3;
    return data->base_quantum * 4;
}

static int get_allotment_for_level(MlfqPolicyData* data, int level) {
    return get_quantum_for_level(data, level) * TIME_ALLOTMENT_RATIO;
}

static void mlfq_tick(void* policy_data) {
    if (!policy_data) return;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;
    data->current_time++;
    
    
    for (int i = 0; i < MAX_PRIORITY_LEVELS - 1; i++) { 
        int size = 0; 
        
        if (!queue_is_empty(data->queues[i])) {
            int count = queue_size(data->queues[i]);
             
            for (int k = 0; k < count; k++) {
                 Process* p = queue_dequeue(data->queues[i]);
                 int wait_time = data->current_time - p->last_active_time;
                 
                 if (wait_time > AGING_THRESHOLD) {
                     
                     p->current_queue_level++;
                     p->current_quantum_runtime = 0;
                     p->time_spent_at_current_level = 0;
                     p->last_active_time = data->current_time; 
                     queue_enqueue(data->queues[p->current_queue_level], p);
                 } else {
                     
                     queue_enqueue(data->queues[i], p);
                 }
            }
        }
    }
}

static bool mlfq_needs_reschedule(void* policy_data, Process* running_process) {
    if (!policy_data) return true;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;

    
    if (running_process == NULL) return true;

    int level = running_process->current_queue_level;

    
    for (int i = MAX_PRIORITY_LEVELS - 1; i > level; i--) {
        if (!queue_is_empty(data->queues[i])) return true;
    }

    
    int q = get_quantum_for_level(data, level);
    if (running_process->current_quantum_runtime >= q) return true;
    
    
    int allotment = get_allotment_for_level(data, level);
    if (running_process->time_spent_at_current_level >= allotment) return true;

    return false;
}

static int mlfq_get_quantum(void* policy_data, Process* process) {
    if (!policy_data || !process) return 0;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;
    return get_quantum_for_level(data, process->current_queue_level);
}

static void mlfq_demote_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;

    int level = process->current_queue_level;

    process->time_spent_at_current_level += process->current_quantum_runtime;
    process->last_active_time = data->current_time; 
    
    int allotment = get_allotment_for_level(data, level);
    int quantum = get_quantum_for_level(data, level);

    bool demote = false;
    if (process->current_quantum_runtime >= quantum) demote = true;
    if (process->time_spent_at_current_level >= allotment) demote = true;

    if (demote && level > 0) {
        level--; 
        process->time_spent_at_current_level = 0; 
    }

    process->current_queue_level = level;
    process->current_quantum_runtime = 0; 
    
    queue_enqueue(data->queues[level], process);
}

static const PolicyVTable mlfq_vtable = {
    .name = "mlfq",
    .create = mlfq_create,
    .destroy = mlfq_destroy,
    .add_process = mlfq_add_process,
    .get_next_process = mlfq_get_next_process,
    .tick = mlfq_tick,
    .needs_reschedule = mlfq_needs_reschedule,
    .get_quantum = mlfq_get_quantum,
    .demote_process = mlfq_demote_process
};

const PolicyVTable* mlfq_get_vtable() {
    return &mlfq_vtable;
}
