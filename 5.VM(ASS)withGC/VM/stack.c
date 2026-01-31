#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

void vm_push(Program *p, Value value) {
    /* prevent writing past the fixed stack */
    if (p->sp >= STACK_MAX) {
        fprintf(stderr, "error: stack overflow\n");
        exit(1);
    }
    p->stack[p->sp++] = value;
}

Value vm_pop(Program *p) {
    /* prevent popping from an empty stack */
    if (p->sp <= 0) {
        fprintf(stderr, "error: stack underflow\n");
        exit(1);
    }
    return p->stack[--p->sp];
}

void vm_push_ret(Program *p, int value) {
    /* prevent writing past the fixed call stack */
    if (p->csp >= STACK_MAX) {
        fprintf(stderr, "error: call stack overflow\n");
        exit(1);
    }
    p->call_stack[p->csp++] = value;
}

int vm_pop_ret(Program *p) {
    /* prevent popping from an empty call stack */
    if (p->csp <= 0) {
        fprintf(stderr, "error: call stack underflow\n");
        exit(1);
    }
    return p->call_stack[--p->csp];
}
