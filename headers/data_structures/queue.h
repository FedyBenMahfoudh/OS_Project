#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#include "process.h"

typedef struct QueueNode QueueNode;

typedef struct Queue Queue;

// Creating a queue : Returns a pointer to an empty queue
Queue* queue_create();

// Inserting a process at the end of the queue
void queue_enqueue(Queue* q, Process* p);

// Pulling a process from the start of the queue (By removing it)
Process* queue_dequeue(Queue* q);

// Peeking at the process that is at the top of the queue (Without removing it)
Process* queue_peek(const Queue* q);

// Verifying if a queue is empty
bool queue_is_empty(const Queue* q);

// Freeing all the memory used by the queue;
void queue_destroy(Queue* q);

#endif