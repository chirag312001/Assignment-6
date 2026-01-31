#ifndef VM_H
#define VM_H

#include "include/value.h"



#define STACK_MAX 1024  /* fixed stack capacity */
#define MEM_SIZE  256   /* global memory slots */
#define VM_EXIT_OK  0   /* normal termination */
#define VM_EXIT_ERR 1   /* runtime / usage error */

/* Program = runtime state of the VM */
typedef struct {
    unsigned char *code;   /* bytecode buffer */
    int code_size;          /* number of bytes */

    int pc;                 /* program counter */

    Value stack[STACK_MAX]; /* operand stack */
    int sp;                 /* next free slot index */

    Value memory[MEM_SIZE]; /* LOAD / STORE memory */

    int call_stack[STACK_MAX]; /* return address stack */
    int csp;                   /* next free slot for call stack */

    int instr_count;         /* instruction count for benchmarks */
} Program;


extern Program *current_program;

/* VM interface */
void vm_init(Program *p, unsigned char *code, int size);
/*free  */
void vm_free(Program *p);
void vm_dump_bytecode(Program *p);

/* Expose GC roots (stack + memory) to the collector. */
void vm_visit_roots(void (*visit)(Obj *));

#endif
