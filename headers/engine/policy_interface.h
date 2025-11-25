// policy_interface.h
#ifndef POLICY_INTERFACE_H
#define POLICY_INTERFACE_H

#include "../data_structures/process.h"

// Handle opaque representing state interne de la politique
typedef struct PolicyHandle PolicyHandle;

// Initialise la politique.
// options: string de la forme "quantum=4;levels=3" (NULL ok)
PolicyHandle* policy_create(const char *options);

// Called when a process becomes ready (arrival).
void policy_on_process_arrival(PolicyHandle *ph, Process *proc, int current_time);

// Called every tick (current_time) — pour aging ou maintenance périodique.
void policy_on_time_tick(PolicyHandle *ph, int current_time);

// Retourne le prochain processus à exécuter (ou NULL si aucun).
Process* policy_select_next(PolicyHandle *ph, int current_time);

// Vérifie si le processus courant doit être préempté.
// Retourne 1 si préemption requise, 0 sinon.
int policy_should_preempt(PolicyHandle *ph, Process *current, int current_time);

// Notifier que le process courant a fini (optional).
void policy_on_process_terminated(PolicyHandle *ph, Process *proc, int current_time);

// Libération des ressources.
void policy_destroy(PolicyHandle *ph);

#endif
