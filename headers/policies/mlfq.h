#ifndef MLFQ_H
#define MLFQ_H

#include "policies.h"

/**
 * @brief Gets the vtable for the MLFQ scheduling policy.
 * @return A constant pointer to the static MLFQ vtable.
 */
const PolicyVTable* mlfq_get_vtable();

#endif
