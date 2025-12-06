#ifndef FIFO_H
#define FIFO_H

#include "../engine/policy_interface.h"

Policy* fifo_policy_create(int quantum);
void fifo_policy_destroy(Policy* policy);
void fifo_policy_add_process(Policy* policy, Process* process);
Process* fifo_policy_get_next_process(Policy* policy);
void fifo_policy_tick(Policy* policy);
bool fifo_policy_needs_reschedule(Policy* policy, Process* running_process);
int fifo_policy_get_quantum(Policy* policy, Process* process);
void fifo_policy_demote_process(Policy* policy, Process* process);

#endif