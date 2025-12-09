#ifndef SRT_H
#define SRT_H

#include "../engine/policy_interface.h"

Policy* srt_policy_create(int quantum);
void srt_policy_destroy(Policy* policy);
void srt_policy_add_process(Policy* policy, Process* process);
Process* srt_policy_get_next_process(Policy* policy);
void srt_policy_tick(Policy* policy);
bool srt_policy_needs_reschedule(Policy* policy, Process* running_process);
int srt_policy_get_quantum(Policy* policy, Process* process);
void srt_policy_demote_process(Policy* policy, Process* process);

#endif