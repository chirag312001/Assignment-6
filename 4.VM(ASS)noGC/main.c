#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "VM/vm.h"
#include "VM/loader.h"
#include "VM/exec.h"


void print_stack(Program *p) {
    printf("\n=== VM HALTED ===\n");

    if (p->sp == 0) {
        printf("Stack is empty\n");
        return;
    }

    printf("Stack (top -> bottom):\n");
    for (int i = p->sp - 1; i >= 0; i--) {
        printf("[%d] %d\n", i, p->stack[i]);
    }
}


int main(int argc, char **argv) {

    /* CLI rule */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <bytecode_file>\n", argv[0]);
        return VM_EXIT_ERR;
    }

    const char *file = argv[1];

    /* enforce .byc extension */
    const char *ext = strrchr(file, '.');
    if (!ext || strcmp(ext, ".byc") != 0) {
        fprintf(stderr, "error: expected .byc file\n");
        return VM_EXIT_ERR;
    }

    /* load bytecode */
    int size = 0;
    unsigned char *code = load_bytecode(file, &size);
    if (!code)
        return VM_EXIT_ERR;

    /* initialize VM program */
    Program prog;
    vm_init(&prog, code, size);

    vm_validate(&prog);
    vm_dump_bytecode(&prog);


    // clocking the execution
    clock_t start = clock();


    vm_run(&prog);


    clock_t end = clock();
    double time_taken = ((double)(end - start))* 1000.0 / CLOCKS_PER_SEC;
    printf("Execution time: %f milliseconds\n", time_taken);
    

    print_stack(&prog);

    vm_free(&prog);
    return VM_EXIT_OK;
}
