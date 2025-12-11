#ifndef PREEMPTIVE_PRIORITY_H
#define PREEMPTIVE_PRIORITY_H

#include "policies.h" 

/**
 * @brief Gets the vtable for the Preemptive Priority scheduling policy.
 * @return A constant pointer to the static Preemptive Priority vtable.
 */
const PolicyVTable* preemptive_priority_get_vtable();

#endif 
