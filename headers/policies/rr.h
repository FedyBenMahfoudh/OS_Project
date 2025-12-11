#ifndef RR_H
#define RR_H

#include "policies.h" 

/**
 * @brief Gets the vtable for the Round Robin (RR) scheduling policy.
 * @return A constant pointer to the static RR vtable.
 */
const PolicyVTable* rr_get_vtable();

#endif  