#include <stdlib.h>
#include <stdbool.h>

#include "data_structures.h"

struct QueueNode {
    Process* process;
    struct QueueNode* next;
};


struct Queue {
    QueueNode* start;
    QueueNode* end;
    int size;
};


// Creating a queue : Returns a pointer to an empty queue
Queue* queue_create() {
    Queue* q = (Queue*) malloc(sizeof(Queue));

    if (!q) return NULL;

    q->start = NULL;
    q->end = NULL;
    q->size = 0;

    return q;
}


// Inserting a process at the end of the queue
void queue_enqueue(Queue* q, Process* p) {
    QueueNode* node = (QueueNode*) malloc(sizeof(QueueNode));
    if (!node) return;

    node->process = p;
    node->next = NULL;

    if (q->size == 0) {
        q->start = node;
        q->end = node;
    } else {
        q->end->next = node;
        q->end = node;
    }

    q->size++;

    return;
}


// Pulling a process from the start of the queue (By removing it)
Process* queue_dequeue(Queue* q) {
    if (!q->start) return NULL;

    QueueNode* node = q->start;
    q->start = q->start->next;
    
    if (q->start == NULL) {
        q->end = NULL;
    }
    
    q->size--;

    Process* p = node->process;
    free(node);
    return p;
}


// Peeking at the process that is at the top of the queue (Without removing it)
Process* queue_peek(const Queue* q) {
    if (!q->start) return NULL;
    return q->start->process;
}


// Verifying if a queue is empty
bool queue_is_empty(const Queue* q) {
    return (q->size == 0);
}

// Freeing all the memory used by the queue;
void queue_destroy(Queue* q) {
    while (!queue_is_empty(q)) {
        queue_dequeue(q);
    }

    free(q);
}