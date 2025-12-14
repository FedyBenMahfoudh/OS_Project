#include "../../headers/engine/policy_interface.h"
#include "../../headers/policies/policies.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POLICIES 10

static const PolicyVTable* policy_registrar[MAX_POLICIES];

static int registered_policy_count = 0;

static const char* policy_names[MAX_POLICIES];

struct Policy {
    const PolicyVTable* vtable; 
    void* concrete_policy_data; 
};

void register_policy(const PolicyVTable* vtable) {
    if (registered_policy_count < MAX_POLICIES) {
        policy_registrar[registered_policy_count] = vtable;
        policy_names[registered_policy_count] = vtable->name;
        registered_policy_count++;
    } else {
        fprintf(stderr, "Policy Registrar Error: Exceeded maximum number of policies (%d).\n", MAX_POLICIES);
    }
}

const char** get_available_policies(int* count) {
    *count = registered_policy_count;
    return (const char**)policy_names;
}

Policy* policy_create(const char* policy_name, int quantum) {
    const PolicyVTable* vtable = NULL;
    for (int i = 0; i < registered_policy_count; i++) {
        if (strcmp(policy_registrar[i]->name, policy_name) == 0) {
            vtable = policy_registrar[i];
            break;
        }
    }

    if (!vtable) {
        fprintf(stderr, "Policy Interface Error: Policy '%s' not recognized or not registered.\n", policy_name);
        return NULL;
    }

    Policy* new_internal_policy = (Policy*)malloc(sizeof(Policy));
    if (!new_internal_policy) {
        perror("Policy Interface: Failed to allocate internal Policy struct");
        return NULL;
    }

    new_internal_policy->vtable = vtable;
    new_internal_policy->concrete_policy_data = vtable->create(quantum);

    if (!new_internal_policy->concrete_policy_data) {
        fprintf(stderr, "Policy Interface Error: Failed to create concrete policy data for '%s'.\n", policy_name);
        free(new_internal_policy);
        return NULL;
    }

    return new_internal_policy;
}

void policy_destroy(Policy* policy) {
    if (!policy) return;
    policy->vtable->destroy(policy->concrete_policy_data);
    free(policy);
}

void policy_add_process(Policy* policy, Process* process) {
    if (!policy || !process) return;
    policy->vtable->add_process(policy->concrete_policy_data, process);
}

Process* policy_get_next_process(Policy* policy) {
    if (!policy) return NULL;
    return policy->vtable->get_next_process(policy->concrete_policy_data);
}

void policy_tick(Policy* policy) {
    if (!policy) return;
    if (policy->vtable->tick) {
        policy->vtable->tick(policy->concrete_policy_data);
    }
}

bool policy_needs_reschedule(Policy* policy, Process* running_process) {
    if (!policy) return true;
    if (policy->vtable->needs_reschedule) {
        return policy->vtable->needs_reschedule(policy->concrete_policy_data, running_process);
    }
    return true;
}

int policy_get_quantum(Policy* policy, Process* process) {
    if (!policy) return 0;
    if (policy->vtable->get_quantum) {
        return policy->vtable->get_quantum(policy->concrete_policy_data, process);
    }
    return 0;
}

void policy_demote_process(Policy* policy, Process* process) {
    if (!policy) return;
    if (policy->vtable->demote_process) {
        policy->vtable->demote_process(policy->concrete_policy_data, process);
    }
}