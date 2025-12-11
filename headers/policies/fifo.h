#ifndef FIFO_H
#define FIFO_H

#include "policies.h"

/**
 * @brief Gets the vtable for the FIFO scheduling policy.
 * @return A constant pointer to the static FIFO vtable.
 */
const PolicyVTable* fifo_get_vtable();

#endif 