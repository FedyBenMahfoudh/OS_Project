#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <stdbool.h>

#include "process.h"

typedef struct MinHeapNode MinHeapNode;

typedef struct MinHeap MinHeap;


typedef int (*Comparator)(Process* a, Process* b);

MinHeap* min_heap_create(Comparator comp);

void min_heap_push(MinHeap* h, Process* p);

Process* min_heap_pop(MinHeap* h);

Process* min_heap_peek(const MinHeap* h);

bool min_heap_is_empty(const MinHeap* h);

void min_heap_destroy(MinHeap* h);

#endif