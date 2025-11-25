// policy_interface.h
#ifndef POLICY_INTERFACE_H
#define POLICY_INTERFACE_H

#include "../data_structures/process.h"

// Opaque Handle representing the intern state of the policy
typedef struct PolicyHandle PolicyHandle;

// Initializing the policy.
// options: string as "quantum=4;levels=3" (NULL ok)
PolicyHandle* policy_create(const char *options);

// Called when a process becomes ready (arrival).
void policy_on_process_arrival(PolicyHandle *ph, Process *proc, int current_time);

// Called every tick (current_time) — For aging.
void policy_on_time_tick(PolicyHandle *ph, int current_time);

// Returns the next process to be executed (or NULL if none).
Process* policy_select_next(PolicyHandle *ph, int current_time);

// Verifies if the currently-running process should be preempted
// Returns 1 if the preeption is required, otherwise 0.
int policy_should_preempt(PolicyHandle *ph, Process *current, int current_time);

// Notifies that the currently-running process finished.
void policy_on_process_terminated(PolicyHandle *ph, Process *proc, int current_time);

// Frees up the resources.
void policy_destroy(PolicyHandle *ph);

#endif
