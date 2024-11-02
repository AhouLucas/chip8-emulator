#include "../headers/stack.h"


stack* init_stack(size_t size) {
    stack* s = malloc(sizeof(stack));
    s->size = size;
    s->arr = malloc(size);
    s->used = 0;
    return s;
}

void free_stack(stack* s) {
    free(s->arr);
    free(s);
    return;
}

void push(stack* s, uint16_t value) {
    if (s->used >= s->size) {
        printf("ERROR : Stack limit reached !");
        exit(EXIT_FAILURE);
    }

    (s->arr)[s->used] = value;
    s->used = s->used + 1;
}

uint16_t pop(stack* s) {
    return (s->arr)[--(s->used)];
}