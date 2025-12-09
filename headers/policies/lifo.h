#ifndef LIFO_H
#define LIFO_H

#include "../engine/policy_interface.h"

Policy* lifo_policy_create(int quantum);
void lifo_policy_destroy(Policy* policy);
void lifo_policy_add_process(Policy* policy, Process* process);
Process* lifo_policy_get_next_process(Policy* policy);
void lifo_policy_tick(Policy* policy);
bool lifo_policy_needs_reschedule(Policy* policy, Process* running_process);
int lifo_policy_get_quantum(Policy* policy, Process* process);
void lifo_policy_demote_process(Policy* policy, Process* process);

#endif