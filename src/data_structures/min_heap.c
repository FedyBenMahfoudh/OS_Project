#include <stdlib.h>
#include <stdbool.h>

#include "../../headers/data_structures/data_structures.h"

#define INITIAL_CAPACITY 16

struct MinHeap {
    Process** data;
    int size;
    int capacity;
};

// Helper function to resize the heap
static void min_heap_resize(MinHeap* h) {
    int new_capacity = h->capacity * 2;
    Process** new_data = realloc(h->data, new_capacity * sizeof(Process*));
    if (new_data) {
        h->data = new_data;
        h->capacity = new_capacity;
    }
}

// Helper function to swap two processes
static void swap(Process** a, Process** b) {
    Process* temp = *a;
    *a = *b;
    *b = temp;
}

// Helper function to maintain heap property going up
static void min_heapify_up(MinHeap* h, int index) {
    if (index == 0) return;

    int parent_index = (index - 1) / 2;
    
    // Min-Heap based on priority (smaller value = higher priority)
    if (h->data[index]->priority < h->data[parent_index]->priority) {
        swap(&h->data[index], &h->data[parent_index]);
        min_heapify_up(h, parent_index);
    }
}

// Helper function to maintain heap property going down
static void min_heapify_down(MinHeap* h, int index) {
    int smallest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < h->size && h->data[left_child]->priority < h->data[smallest]->priority) {
        smallest = left_child;
    }

    if (right_child < h->size && h->data[right_child]->priority < h->data[smallest]->priority) {
        smallest = right_child;
    }

    if (smallest != index) {
        swap(&h->data[index], &h->data[smallest]);
        min_heapify_down(h, smallest);
    }
}

// Creating a min heap : Returns a pointer to an empty min heap
MinHeap* min_heap_create() {
    MinHeap* h = (MinHeap*) malloc(sizeof(MinHeap));
    if (!h) return NULL;

    h->data = (Process**) malloc(INITIAL_CAPACITY * sizeof(Process*));
    if (!h->data) {
        free(h);
        return NULL;
    }

    h->size = 0;
    h->capacity = INITIAL_CAPACITY;

    return h;
}

// Adding a process to the min heap
void min_heap_push(MinHeap* h, Process* p) {
    if (h->size == h->capacity) {
        min_heap_resize(h);
    }

    h->data[h->size] = p;
    min_heapify_up(h, h->size);
    h->size++;
}

// Pulling a process from the min heap (By removing it)
Process* min_heap_pop(MinHeap* h) {
    if (h->size == 0) return NULL;

    Process* root = h->data[0];
    
    // Move last element to root
    h->data[0] = h->data[h->size - 1];
    h->size--;

    if (h->size > 0) {
        min_heapify_down(h, 0);
    }

    return root;
}

// Peeking at the process that is at the top of the min heap (Without removing it)
Process* min_heap_peek(const MinHeap* h) {
    if (h->size == 0) return NULL;
    return h->data[0];
}

// Verifying if a min heap is empty
bool min_heap_is_empty(const MinHeap* h) {
    return (h->size == 0);
}

// Freeing all the memory used by the min heap;
void min_heap_destroy(MinHeap* h) {
    if (h) {
        if (h->data) {
            free(h->data);
        }
        free(h);
    }
}
