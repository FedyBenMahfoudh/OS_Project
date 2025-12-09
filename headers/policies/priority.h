#ifndef PRIORITY_H
#define PRIORITY_H

#include "../engine/policy_interface.h"

Policy* priority_policy_create(int quantum);
void priority_policy_destroy(Policy* policy);
void priority_policy_add_process(Policy* policy, Process* process);
Process* priority_policy_get_next_process(Policy* policy);
void priority_policy_tick(Policy* policy);
bool priority_policy_needs_reschedule(Policy* policy, Process* running_process);
int priority_policy_get_quantum(Policy* policy, Process* process);
void priority_policy_demote_process(Policy* policy, Process* process);

#endif