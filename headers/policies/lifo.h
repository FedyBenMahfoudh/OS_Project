#ifndef LIFO_H
#define LIFO_H

#include "../engine/policy_interface.h"

Policy* lifo_policy_create(int quantum);
void lifo_policy_destroy(Policy* policy);
void lifo_add_process(Policy* policy, Process* process);
Process* lifo_get_next_process(Policy* policy);

#endif