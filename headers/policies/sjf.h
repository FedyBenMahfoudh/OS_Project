#ifndef SJF_H
#define SJF_H

#include "../engine/policy_interface.h"

Policy* sjf_policy_create(int quantum);
void sjf_policy_destroy(Policy* policy);
void sjf_add_process(Policy* policy, Process* process);
Process* sjf_get_next_process(Policy* policy);

#endif