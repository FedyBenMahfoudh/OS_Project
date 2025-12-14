#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

#include "process.h"

typedef struct StackNode StackNode;

typedef struct Stack Stack;

Stack* stack_create();

void stack_push(Stack* s, Process* p);

Process* stack_pop(Stack* s);

Process* stack_peek(const Stack* s);

bool stack_is_empty(const Stack* s);

void stack_destroy(Stack* s);

#endif