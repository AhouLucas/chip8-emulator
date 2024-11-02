#ifndef STACK_H
#define STACK_H

#include "common.h"

typedef struct {
    uint16_t* arr;
    size_t size;
    size_t used;
} stack;

stack* init_stack(size_t);
void free_stack(stack*);
void push(stack*, uint16_t);
uint16_t pop(stack*);

#endif