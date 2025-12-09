#ifndef RR_H
#define RR_H

#include "../engine/policy_interface.h"

Policy* rr_policy_create(int quantum);
void rr_policy_destroy(Policy* policy);
void rr_policy_add_process(Policy* policy, Process* process);
Process* rr_policy_get_next_process(Policy* policy);

void rr_policy_tick(Policy* policy);
bool rr_policy_needs_reschedule(Policy* policy, Process* running_process);
int rr_policy_get_quantum(Policy* policy, Process* process);
void rr_policy_demote_process(Policy* policy, Process* process);

#endif