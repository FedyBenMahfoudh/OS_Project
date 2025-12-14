#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#include "process.h"

typedef struct QueueNode QueueNode;

typedef struct Queue Queue;

Queue* queue_create();

void queue_enqueue(Queue* q, Process* p);

Process* queue_dequeue(Queue* q);

Process* queue_peek(const Queue* q);

bool queue_is_empty(const Queue* q);

int queue_size(const Queue* q);

void queue_destroy(Queue* q);

#endif