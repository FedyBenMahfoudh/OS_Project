#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include <stdbool.h>

#include "process.h"

typedef struct MaxHeapNode MaxHeapNode;

typedef struct MaxHeap MaxHeap;


typedef int (*Comparator)(Process* a, Process* b);

MaxHeap* max_heap_create(Comparator comp);

void max_heap_push(MaxHeap* h, Process* p);

Process* max_heap_pop(MaxHeap* h);

Process* max_heap_peek(const MaxHeap* h);

bool max_heap_is_empty(const MaxHeap* h);

void max_heap_destroy(MaxHeap* h);

#endif