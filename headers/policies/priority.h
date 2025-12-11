#ifndef PRIORITY_H
#define PRIORITY_H

#include "policies.h" 

/**
 * @brief Gets the vtable for the Priority scheduling policy.
 * @return A constant pointer to the static Priority vtable.
 */
const PolicyVTable* priority_get_vtable();

#endif