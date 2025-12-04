#ifndef SJF_H
#define SJF_H

#include "../engine/policy_interface.h"

Policy* sjf_policy_create(int quantum);
void sjf_policy_destroy(Policy* policy);
void sjf_policy_add_process(Policy* policy, Process* process);
Process* sjf_policy_get_next_process(Policy* policy);
void sjf_policy_tick(Policy* policy);
bool sjf_policy_needs_reschedule(Policy* policy, Process* running_process);

#endif