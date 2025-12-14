#ifndef POLICY_INTERFACE_H
#define POLICY_INTERFACE_H

#include "../data_structures/process.h"

typedef enum {
    POLICY_TYPE_NONE = 0,
    POLICY_TYPE_FIFO,
    POLICY_TYPE_LIFO,
    POLICY_TYPE_SJF,
    POLICY_TYPE_PRIORITY,
    POLICY_TYPE_RR,
    POLICY_TYPE_SRT,
    POLICY_TYPE_MLFQ
} PolicyType;

typedef struct Policy Policy;

Policy* policy_create(const char* policy_name, int quantum);

void policy_destroy(Policy* policy);

void policy_add_process(Policy* policy, Process* process);

Process* policy_get_next_process(Policy* policy);

void policy_tick(Policy* policy);

bool policy_needs_reschedule(Policy* policy, Process* running_process);

int policy_get_quantum(Policy* policy, Process* process);

void policy_demote_process(Policy* policy, Process* process);

#endif 
