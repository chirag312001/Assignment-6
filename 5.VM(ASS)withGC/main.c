#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "VM/vm.h"
#include "VM/loader.h"
#include "VM/exec.h"
#include "VM/include/object.h"   /* for gc_collect */
#include "debugger/debugger.h"


void print_stack(Program *p) {
    printf("\n=== VM HALTED ===\n");

    if (p->sp == 0) {
        printf("Stack is empty\n");
        return;
    }

    printf("Stack (top -> bottom):\n");
    for (int i = p->sp - 1; i >= 0; i--) {

        Value v = p->stack[i];

        if (v.type == VAL_OBJ && v.obj != NULL) {

            if (v.obj->type == OBJ_INT) {
                ObjInt *oi = (ObjInt *)v.obj;
                printf("[%d] ObjInt value=%d\n", i, oi->value);
            }
            else if (v.obj->type == OBJ_PAIR) {
                printf("[%d] ObjPair %p\n", i, (void *)v.obj);
            }
            else {
                printf("[%d] Obj(type=%d) %p\n", i, v.obj->type, (void *)v.obj);
            }

        } else {
            printf("[%d] <invalid>\n", i);
        }
    }
}



void print_memory(Program *p) {
    printf("\n=== GLOBAL MEMORY (VARIABLES) ===\n");
    int empty = 1;

    for (int i = 0; i < MEM_SIZE; i++) {
        Value v = p->memory[i];
        if (v.type == VAL_NIL) continue;

        empty = 0;
        printf("[%d] ", i);
        if (v.type == VAL_OBJ && v.obj != NULL) {
            if (v.obj->type == OBJ_INT) printf("Int: %d\n", ((ObjInt *)v.obj)->value);
            else if (v.obj->type == OBJ_PAIR) printf("Pair: %p\n", (void *)v.obj);
            else printf("Obj(type=%d)\n", v.obj->type);
        } else {
            printf("<Invalid>\n");
        }
    }
    if (empty) printf("(Memory is empty)\n");
    printf("=================================\n");
}


// int main(int argc, char **argv) {

//     /* CLI rule */
//     if (argc != 2) {
//         fprintf(stderr, "usage: %s <bytecode_file>\n", argv[0]);
//         return 1;
//     }

//     const char *file = argv[1];

//     /* enforce .byc extension */
//     const char *ext = strrchr(file, '.');
//     if (!ext || strcmp(ext, ".byc") != 0) {
//         fprintf(stderr, "error: expected .byc file\n");
//         return 1;
//     }

//     /* load bytecode */
//     int size = 0;
//     unsigned char *code = load_bytecode(file, &size);
//     if (!code)
//         return 1;

//     /* initialize VM program */
//     Program prog;
//     vm_init(&prog, code, size);

//     vm_validate(&prog);
//     vm_dump_bytecode(&prog);


//     // clocking the execution
//     clock_t start = clock();


//     vm_run(&prog);


//     clock_t end = clock();
//     double time_taken = ((double)(end - start))* 1000.0 / CLOCKS_PER_SEC;
//     printf("Execution time: %f milliseconds\n", time_taken);
    
//     printf("\n===== GC WHILE STACK IS LIVE =====\n");

//     no_of_object_freed = 0;
//      /* Start GC timer */
//     clock_t gc_start = clock();

//     /* Run GC */
//     gc_collect(0);  /* show debug info */

//     /* End GC timer */
//     clock_t gc_end = clock();
//     double gc_time_taken = ((double)(gc_end - gc_start)) * 1000.0 / CLOCKS_PER_SEC;

//     /* Print GC stats */
//     printf("\nGC SUMMARY\n");
//     printf("Objects freed: %d\n", no_of_object_freed);
//     printf("GC time: %f milliseconds\n", gc_time_taken);

//     print_stack(&prog);

//     print_memory(&prog);
    
//     // printf("\n===== GC AFTER VM RUN =====\n");
//     // gc_collect();

//     vm_free(&prog);
//     return 0;
// }


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <bytecode_file> [debug]\n", argv[0]);
        return 1;
    }

    const char *file = argv[1];
    int is_debug = (argc == 3 && strcmp(argv[2], "debug") == 0);

    /* enforce .byc extension */
    const char *ext = strrchr(file, '.');
    if (!ext || strcmp(ext, ".byc") != 0) {
        fprintf(stderr, "error: expected .byc file\n");
        return 1;
    }

    int size = 0;
    unsigned char *code = load_bytecode(file, &size);
    if (!code) return 1;

    Program prog;
    vm_init(&prog, code, size);

    if (is_debug) {
        // --- PHASE 1 START ---
        debug_start(&prog);
        // --- PHASE 1 END ---
    } else {
        // Standard Execution
        vm_validate(&prog);
        vm_run(&prog);
        gc_collect(0);
        print_stack(&prog);
        print_memory(&prog);
    }

    vm_free(&prog);
    return 0;
}