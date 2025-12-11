#ifndef SRT_H
#define SRT_H

#include "policies.h"

/**
 * @brief Gets the vtable for the SRT scheduling policy.
 * @return A constant pointer to the static SRT vtable.
 */
const PolicyVTable* srt_get_vtable();

#endif