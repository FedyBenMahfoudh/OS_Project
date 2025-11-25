#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

#include "process.h"

typedef struct StackNode StackNode;

typedef struct Stack Stack;

// Creating a stack : Returns a pointer to an empty stack
Stack* stack_create();

// Inserting a process at the top of the stack
void stack_push(Stack* s, Process* p);

// Pulling a process from the top of the stack (By removing it)
Process* stack_pop(Stack* s);

// Peeking at the process that is at the top of the stack (Without removing it)
Process* stack_peek(const Stack* s);

// Verifying if a stack is empty
bool stack_is_empty(const Stack* s);

// Freeing all the memory used by the stack;
void stack_destroy(Stack* s);

#endif