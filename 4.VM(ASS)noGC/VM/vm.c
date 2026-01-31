#include "vm.h"
#include <stdlib.h>
#include <stdio.h>

void vm_init(Program *p, unsigned char *code, int size) {
    p->code = code;
    p->code_size = size;
    p->pc = 0;
    p->sp = 0;
    p->csp = 0;
    p->instr_count = 0;

    /* clear memory so LOAD reads predictable values */
    for (int i = 0; i < MEM_SIZE; i++)
        p->memory[i] = 0;
}

void vm_free(Program *p) {
    /* VM owns bytecode memory */
    free(p->code);
}

void vm_dump_bytecode(Program *p) {
    printf("Bytecode dump (%d bytes):\n", p->code_size);

    for (int i = 0; i < p->code_size; i++) {
        printf("%02X ", p->code[i]);

        /* new line every 8 bytes */
        if ((i + 1) % 8 == 0)
            printf("\n");
    }

    printf("\n");
}

