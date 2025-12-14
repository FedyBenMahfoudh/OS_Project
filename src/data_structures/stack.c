#include <stdlib.h>
#include <stdbool.h>

#include "../../headers/data_structures/data_structures.h"

struct StackNode {
    Process* process;
    struct StackNode* next;
};

struct Stack {
    StackNode* top;
    int size;
};

Stack* stack_create() {
    Stack* s = malloc(sizeof(Stack));

    if (!s) return NULL;

    s->top = NULL;
    s->size = 0;

    return s;
}

void stack_push(Stack* s, Process* p) {
    StackNode* node = (StackNode*) malloc(sizeof(StackNode));
    if (!node) return;

    node->process = p;
    node->next = s->top;
    s->top = node;

    s->size++;

    return;
}

Process* stack_pop(Stack* s) {
    if (!s->top) return NULL;

    StackNode* node = s->top;
    s->top = s->top->next;
    s->size--;

    Process* p = node->process;
    free(node);
    return p;
}

Process* stack_peek(const Stack* s) {
    if (!s->top) return NULL;
    return s->top->process;
}

bool stack_is_empty(const Stack* s) {
    return (s->size == 0);
}

void stack_destroy(Stack* s) {
    while (!stack_is_empty(s)) {
        stack_pop(s);
    }

    free(s);
}