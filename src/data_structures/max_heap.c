#include <stdlib.h>
#include <stdbool.h>

#include "../../headers/data_structures/data_structures.h"

#define INITIAL_CAPACITY 16

struct MaxHeap {
    Process** data;
    int size;
    int capacity;
    Comparator comp;
};

static void max_heap_resize(MaxHeap* h) {
    int new_capacity = h->capacity * 2;
    Process** new_data = realloc(h->data, new_capacity * sizeof(Process*));
    if (new_data) {
        h->data = new_data;
        h->capacity = new_capacity;
    }
}

static void swap(Process** a, Process** b) {
    Process* temp = *a;
    *a = *b;
    *b = temp;
}

static void max_heapify_up(MaxHeap* h, int index) {
    if (index == 0) return;

    int parent_index = (index - 1) / 2;

    if (h->comp(h->data[index], h->data[parent_index]) > 0) {
        swap(&h->data[index], &h->data[parent_index]);
        max_heapify_up(h, parent_index);
    }
}

static void max_heapify_down(MaxHeap* h, int index) {
    int largest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < h->size && h->comp(h->data[left_child], h->data[largest]) > 0) {
        largest = left_child;
    }

    if (right_child < h->size && h->comp(h->data[right_child], h->data[largest]) > 0) {
        largest = right_child;
    }

    if (largest != index) {
        swap(&h->data[index], &h->data[largest]);
        max_heapify_down(h, largest);
    }
}

MaxHeap* max_heap_create(Comparator comp) {
    MaxHeap* h = (MaxHeap*) malloc(sizeof(MaxHeap));
    if (!h) return NULL;

    h->data = (Process**) malloc(INITIAL_CAPACITY * sizeof(Process*));
    if (!h->data) {
        free(h);
        return NULL;
    }

    h->size = 0;
    h->capacity = INITIAL_CAPACITY;
    h->comp = comp;

    return h;
}

void max_heap_push(MaxHeap* h, Process* p) {
    if (h->size == h->capacity) {
        max_heap_resize(h);
    }

    h->data[h->size] = p;
    max_heapify_up(h, h->size);
    h->size++;
}

Process* max_heap_pop(MaxHeap* h) {
    if (h->size == 0) return NULL;

    Process* root = h->data[0];

    h->data[0] = h->data[h->size - 1];
    h->size--;

    if (h->size > 0) {
        max_heapify_down(h, 0);
    }

    return root;
}

Process* max_heap_peek(const MaxHeap* h) {
    if (h->size == 0) return NULL;
    return h->data[0];
}

bool max_heap_is_empty(const MaxHeap* h) {
    return (h->size == 0);
}

void max_heap_destroy(MaxHeap* h) {
    if (h) {
        if (h->data) {
            free(h->data);
        }
        free(h);
    }
}
