#include "../../headers/policies/mlfq.h"
#include "../../headers/data_structures/queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_PRIORITY_LEVELS 20
#define AGING_THRESHOLD 10  // Ticks before promotion
#define TIME_ALLOTMENT_RATIO 5 // Allotment = Quantum * Ratio

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
    
    // Initial placement: Config priority or clamped to MAX-1
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

    // Scan from highest priority (MAX-1) down to 0
    for (int i = MAX_PRIORITY_LEVELS - 1; i >= 0; i--) {
        if (!queue_is_empty(data->queues[i])) {
            Process* p = queue_dequeue(data->queues[i]);
            // Update last active time for aging check later
            // But real update happens when it runs
            return p;
        }
    }
    return NULL;
}

// Helper: Calculate Quantum for a level
// Higher level = Higher priority = Shorter Quantum
// Level 0 (Lowest) = Longest Quantum
// Formula: Q = base * 2^(MAX - 1 - level)
// But to keep it reasonable, let's just do base * (MAX - level) or similar
// Or simpler: Top half gets base, Bottom half gets base*2, etc.
// Let's stick to the user's plan: "Top = Short, Bottom = Long"
// Q_i = base * 2^(MAX_LEVELS - 1 - i) might be too huge.
// Let's use a simpler exponential scale but clamped.
// 20 levels total. Split into 4 tiers of 5 levels each.
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
    
    // Aging Check: Iterate all queues except the top one
    for (int i = 0; i < MAX_PRIORITY_LEVELS - 1; i++) {
        // We can't iterate the queue easily without exposing internals or de-queueing
        // This is a limitation of the Queue ADT.
        // PROPER APPROACH for Queue ADT: 
        // We can only check the head. If head is starving, move it. 
        // Or we assume the "Process" struct is updated externally? No.
        
        // Workaround: Rotate the queue? Expensive.
        // Efficient Workaround: Check Head. If head is old, promote. Repeat until head is young.
        
        int size = 0; // Unknown size... 
        // Since we can't iterate, we will trust the Scheduler Engine calls us enough.
        // Actually, we can peek/rotate if we modify Queue, but we are in policy.
        
        // Optimization: Only check queues that have items.
        // Since we lack a 'size' or iterator, we will do a "Best Effort" aging on the head.
        if (!queue_is_empty(data->queues[i])) {
            int count = queue_size(data->queues[i]);
            // Full Scan: Process exactly 'count' items.
            // Items that are not promoted are re-enqueued.
            // Items that are promoted are moved to higher queue.
            // This preserves the relative order of the non-promoted items (rotation).
            // Since we rotate everyone, the order A,B,C -> B,C,A -> C,A,B -> A,B,C is restored (if none promoted)
            
            for (int k = 0; k < count; k++) {
                 Process* p = queue_dequeue(data->queues[i]);
                 int wait_time = data->current_time - p->last_active_time;
                 
                 if (wait_time > AGING_THRESHOLD) {
                     // Promote!
                     p->current_queue_level++;
                     p->current_quantum_runtime = 0;
                     p->time_spent_at_current_level = 0;
                     p->last_active_time = data->current_time; // Reset aging timer
                     queue_enqueue(data->queues[p->current_queue_level], p);
                 } else {
                     // Not old enough, put back
                     queue_enqueue(data->queues[i], p);
                 }
            }
        }
    }
}

static bool mlfq_needs_reschedule(void* policy_data, Process* running_process) {
    if (!policy_data) return true;
    MlfqPolicyData* data = (MlfqPolicyData*)policy_data;

    // 1. If idle, reschedule
    if (running_process == NULL) return true;

    int level = running_process->current_queue_level;

    // 2. Preemption: Check if ANY higher queue has a process
    for (int i = MAX_PRIORITY_LEVELS - 1; i > level; i--) {
        if (!queue_is_empty(data->queues[i])) return true;
    }

    // 3. Quantum / Allotment Expiration
    int q = get_quantum_for_level(data, level);
    if (running_process->current_quantum_runtime >= q) return true;
    
    // 4. Anti-Gaming Allotment check (though demotion happens in demote_process)
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
    
    // Update total time spent at this level
    // This function is called when a process yields or is preempted
    process->time_spent_at_current_level += process->current_quantum_runtime;
    process->last_active_time = data->current_time; // It just ran, so it's active
    
    int allotment = get_allotment_for_level(data, level);
    int quantum = get_quantum_for_level(data, level);
    
    // Check Demotion Criteria:
    // 1. Used up full quantum in one go? (Normal RR logic)
    // 2. Used up total allotment? (Anti-gaming logic)
    
    bool demote = false;
    if (process->current_quantum_runtime >= quantum) demote = true;
    if (process->time_spent_at_current_level >= allotment) demote = true;

    if (demote && level > 0) {
        level--; // Moves down (remember 0 is bottom)
        process->time_spent_at_current_level = 0; // Reset allotment tracking for new level
    }

    process->current_queue_level = level;
    process->current_quantum_runtime = 0; // Reset runtime
    
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
