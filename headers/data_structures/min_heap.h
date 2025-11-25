#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <stdbool.h>

#include "process.h"

typedef struct MinHeapNode MinHeapNode;

typedef struct MinHeap MinHeap;


// Creating a min heap : Returns a pointer to an empty min heap
MinHeap* min_heap_create();

// Adding a process to the min heap
void min_heap_push(MinHeap* h, Process* p);

// Pulling a process from the min heap (By removing it)
Process* min_heap_pop(MinHeap* h);

// Peeking at the process that is at the top of the min heap (Without removing it)
Process* min_heap_peek(const MinHeap* h);

// Verifying if a min heap is empty
bool min_heap_is_empty(const MinHeap* h);

// Freeing all the memory used by the min heap;
void min_heap_destroy(MinHeap* h);

#endif