#include <stdlib.h>
#include <stdbool.h>

#include "data_structures.h"

struct StackNode {
    Process* process;
    struct StackNode* next;
};


struct Stack {
    StackNode* top;
    int size;
};


// Creating a stack : Returns a pointer to an empty stack
Stack* stack_create() {
    Stack* s = malloc(sizeof(Stack));

    if (!s) return NULL;

    s->top = NULL;
    s->size = 0;

    return s;
}


// Inserting a process at the top of the stack
void stack_push(Stack* s, Process* p) {
    StackNode* node = (StackNode*) malloc(sizeof(StackNode));
    if (!node) return;

    node->process = p;
    node->next = s->top;
    s->top = node;

    s->size++;

    return;
}

// Pulling a process from the top of the stack (By removing it)
Process* stack_pop(Stack* s) {
    if (!s->top) return NULL;

    StackNode* node = s->top;
    s->top = s->top->next;
    s->size--;

    Process* p = node->process;
    free(node);
    return p;
}

// Peeking at the process that is at the top of the stack (Without removing it)
Process* stack_peek(const Stack* s) {
    if (!q->top) return NULL;
    return s->top->process;
}

// Verifying if a stack is empty
bool stack_is_empty(const Stack* s) {
    return (s->size == 0);
}

// Freeing all the memory used by the stack;
void stack_destroy(Stack* s) {
    while (!stack_is_empty(s)) {
        stack_pop(s);
    }

    free(s);
}