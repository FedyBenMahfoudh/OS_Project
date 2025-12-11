#include "../../headers/policies/fifo.h"
#include "../../headers/data_structures/queue.h"
#include <stdlib.h>

// --- Internal FIFO Policy Data Structure ---
typedef struct {
    Queue* queue;
} FifoPolicyData;

// --- Static (Private) Policy Functions ---

static void* fifo_create(int quantum) {
    // Ignoring the quantum
    (void)quantum; 
    FifoPolicyData* policy_data = (FifoPolicyData*)malloc(sizeof(FifoPolicyData));
    if (!policy_data) return NULL;

    policy_data->queue = queue_create();
    if (!policy_data->queue) {
        free(policy_data);
        return NULL;
    }
    return policy_data;
}

static void fifo_destroy(void* policy_data) {
    if (!policy_data) return;
    FifoPolicyData* fifo_data = (FifoPolicyData*)policy_data;
    queue_destroy(fifo_data->queue);
    free(fifo_data);
}

static void fifo_add_process(void* policy_data, Process* process) {
    if (!policy_data || !process) return;
    FifoPolicyData* fifo_data = (FifoPolicyData*)policy_data;
    queue_enqueue(fifo_data->queue, process);
}

static Process* fifo_get_next_process(void* policy_data) {
    if (!policy_data) return NULL;
    FifoPolicyData* fifo_data = (FifoPolicyData*)policy_data;
    if (queue_is_empty(fifo_data->queue)) return NULL;
    return queue_dequeue(fifo_data->queue);
}

static void fifo_tick(void* policy_data) {
    (void)policy_data; // No-op for FIFO
}

static bool fifo_needs_reschedule(void* policy_data, Process* running_process) {
    (void)policy_data;
    // When the CPU is idle.
    return running_process == NULL;
}

static int fifo_get_quantum(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process;
    return 0; 
}

static void fifo_demote_process(void* policy_data, Process* process) {
    (void)policy_data;
    (void)process; 
}

// --- VTable Definition ---

static const PolicyVTable fifo_vtable = {
    .name = "fifo",
    .create = fifo_create,
    .destroy = fifo_destroy,
    .add_process = fifo_add_process,
    .get_next_process = fifo_get_next_process,
    .tick = fifo_tick,
    .needs_reschedule = fifo_needs_reschedule,
    .get_quantum = fifo_get_quantum,
    .demote_process = fifo_demote_process
};

// --- Public VTable Accessor ---

const PolicyVTable* fifo_get_vtable() {
    return &fifo_vtable;
}
