#ifndef POLICIES_H
#define POLICIES_H

#include <stdbool.h>
#include "../data_structures/process.h"

typedef struct Policy Policy;

typedef struct PolicyVTable {
    const char* name;
    void* (*create)(int quantum);
    void (*destroy)(void* policy_data);
    void (*add_process)(void* policy_data, Process* process);
    Process* (*get_next_process)(void* policy_data);
    void (*tick)(void* policy_data);
    bool (*needs_reschedule)(void* policy_data, Process* running_process);
    int (*get_quantum)(void* policy_data, Process* process);
    void (*demote_process)(void* policy_data, Process* process);
} PolicyVTable;

void register_policy(const PolicyVTable* vtable);

const char** get_available_policies(int* count);

void register_all_policies();

#endif
