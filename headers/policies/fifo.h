#ifndef FIFO_H
#define FIFO_H

#include "../engine/policy_interface.h"

Policy* fifo_policy_create(int quantum);
void fifo_policy_destroy(Policy* policy);
void fifo_add_process(Policy* policy, Process* process);
Process* fifo_get_next_process(Policy* policy);

#endif