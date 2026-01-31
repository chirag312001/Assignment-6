#ifndef STACK_H
#define STACK_H

#include "vm.h"

/* operand stack helpers with safety checks */
void vm_push(Program *p, int value);
int vm_pop(Program *p);

/* call stack helpers with safety checks */
void vm_push_ret(Program *p, int value);
int vm_pop_ret(Program *p);

#endif
