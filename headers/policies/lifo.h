#ifndef LIFO_H
#define LIFO_H

#include "policies.h" 

/**
 * @brief Gets the vtable for the LIFO scheduling policy.
 * @return A constant pointer to the static LIFO vtable.
 */
const PolicyVTable* lifo_get_vtable();

#endif 