#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include <stdbool.h>

#include "process.h"

typedef struct MaxHeapNode MaxHeapNode;

typedef struct MaxHeap MaxHeap;


// Comparator function type: returns <0 if a < b, 0 if a == b, >0 if a > b
typedef int (*Comparator)(Process* a, Process* b);

// Creating a max heap : Returns a pointer to an empty max heap
MaxHeap* max_heap_create(Comparator comp);

// Adding a process to the max heap
void max_heap_push(MaxHeap* h, Process* p);

// Pulling a process from the max heap (By removing it)
Process* max_heap_pop(MaxHeap* h);

// Peeking at the process that is at the top of the max heap (Without removing it)
Process* max_heap_peek(const MaxHeap* h);

// Verifying if a max heap is empty
bool max_heap_is_empty(const MaxHeap* h);

// Freeing all the memory used by the max heap;
void max_heap_destroy(MaxHeap* h);

#endif