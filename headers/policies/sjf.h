#ifndef SJF_H
#define SJF_H

#include "policies.h"

/**
 * @brief Gets the vtable for the SJF scheduling policy.
 * @return A constant pointer to the static SJF vtable.
 */
const PolicyVTable* sjf_get_vtable();

#endif 