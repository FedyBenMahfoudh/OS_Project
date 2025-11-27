#ifndef PRIORITY_H
#define PRIORITY_H

#include "../engine/policy_interface.h"

Policy* priority_policy_create(int quantum);
void priority_policy_destroy(Policy* policy);
void priority_add_process(Policy* policy, Process* process);
Process* priority_get_next_process(Policy* policy);

#endif