#include <stdlib.h>
#include <stdbool.h>
#include "data_structures.h"

#define INITIAL_CAPACITY 16

struct MaxHeap {
    Process** data;
    int size;
    int capacity;
};

// Helper function to resize the heap
static void max_heap_resize(MaxHeap* h) {
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
static void max_heapify_up(MaxHeap* h, int index) {
    if (index == 0) return;

    int parent_index = (index - 1) / 2;
    
    // Max-Heap based on priority (larger value = higher priority)
    if (h->data[index]->priority > h->data[parent_index]->priority) {
        swap(&h->data[index], &h->data[parent_index]);
        max_heapify_up(h, parent_index);
    }
}

// Helper function to maintain heap property going down
static void max_heapify_down(MaxHeap* h, int index) {
    int largest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < h->size && h->data[left_child]->priority > h->data[largest]->priority) {
        largest = left_child;
    }

    if (right_child < h->size && h->data[right_child]->priority > h->data[largest]->priority) {
        largest = right_child;
    }

    if (largest != index) {
        swap(&h->data[index], &h->data[largest]);
        max_heapify_down(h, largest);
    }
}

// Creating a max heap : Returns a pointer to an empty max heap
MaxHeap* max_heap_create() {
    MaxHeap* h = (MaxHeap*) malloc(sizeof(MaxHeap));
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

// Adding a process to the max heap
void max_heap_push(MaxHeap* h, Process* p) {
    if (h->size == h->capacity) {
        max_heap_resize(h);
    }

    h->data[h->size] = p;
    max_heapify_up(h, h->size);
    h->size++;
}

// Pulling a process from the max heap (By removing it)
Process* max_heap_pop(MaxHeap* h) {
    if (h->size == 0) return NULL;

    Process* root = h->data[0];
    
    // Move last element to root
    h->data[0] = h->data[h->size - 1];
    h->size--;

    if (h->size > 0) {
        max_heapify_down(h, 0);
    }

    return root;
}

// Peeking at the process that is at the top of the max heap (Without removing it)
Process* max_heap_peek(const MaxHeap* h) {
    if (h->size == 0) return NULL;
    return h->data[0];
}

// Verifying if a max heap is empty
bool max_heap_is_empty(const MaxHeap* h) {
    return (h->size == 0);
}

// Freeing all the memory used by the max heap;
void max_heap_destroy(MaxHeap* h) {
    if (h) {
        if (h->data) {
            free(h->data);
        }
        free(h);
    }
}
