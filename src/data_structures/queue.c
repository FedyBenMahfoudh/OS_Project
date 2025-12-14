#include <stdlib.h>
#include <stdbool.h>

#include "../../headers/data_structures/data_structures.h"

struct QueueNode {
    Process* process;
    struct QueueNode* next;
};


struct Queue {
    QueueNode* start;
    QueueNode* end;
    int size;
};

Queue* queue_create() {
    Queue* q = (Queue*) malloc(sizeof(Queue));

    if (!q) return NULL;

    q->start = NULL;
    q->end = NULL;
    q->size = 0;

    return q;
}

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

Process* queue_peek(const Queue* q) {
    if (!q->start) return NULL;
    return q->start->process;
}

bool queue_is_empty(const Queue* q) {
    return (q->size == 0);
}
int queue_size(const Queue* q) {
    return q ? q->size : 0;
}
void queue_destroy(Queue* q) {
    while (!queue_is_empty(q)) {
        queue_dequeue(q);
    }

    free(q);
}