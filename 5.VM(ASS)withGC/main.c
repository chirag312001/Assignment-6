#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "VM/vm.h"
#include "VM/loader.h"
#include "VM/exec.h"
#include "VM/include/object.h"   /* for gc_collect */


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


int main(int argc, char **argv) {

    /* CLI rule */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <bytecode_file>\n", argv[0]);
        return 1;
    }

    const char *file = argv[1];

    /* enforce .byc extension */
    const char *ext = strrchr(file, '.');
    if (!ext || strcmp(ext, ".byc") != 0) {
        fprintf(stderr, "error: expected .byc file\n");
        return 1;
    }

    /* load bytecode */
    int size = 0;
    unsigned char *code = load_bytecode(file, &size);
    if (!code)
        return 1;

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
    
    printf("\n===== GC WHILE STACK IS LIVE =====\n");

    no_of_object_freed = 0;
     /* Start GC timer */
    clock_t gc_start = clock();

    /* Run GC */
    gc_collect(0);  /* show debug info */

    /* End GC timer */
    clock_t gc_end = clock();
    double gc_time_taken = ((double)(gc_end - gc_start)) * 1000.0 / CLOCKS_PER_SEC;

    /* Print GC stats */
    printf("\nGC SUMMARY\n");
    printf("Objects freed: %d\n", no_of_object_freed);
    printf("GC time: %f milliseconds\n", gc_time_taken);

    print_stack(&prog);

    print_memory(&prog);
    
    // printf("\n===== GC AFTER VM RUN =====\n");
    // gc_collect();

    vm_free(&prog);
    return 0;
}



// // 5.VM(ASS)withGC/main.c

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>

// #include "VM/vm.h"
// #include "VM/loader.h"
// #include "VM/exec.h"
// #include "VM/include/object.h" 

// void print_memory(Program *p) {
//     printf("\n=== GLOBAL MEMORY (VARIABLES) ===\n");
//     int empty = 1;

//     for (int i = 0; i < MEM_SIZE; i++) {
//         Value v = p->memory[i];
//         if (v.type == VAL_NIL) continue;

//         empty = 0;
//         printf("[%d] ", i);
//         if (v.type == VAL_OBJ && v.obj != NULL) {
//             if (v.obj->type == OBJ_INT) printf("Int: %d\n", ((ObjInt *)v.obj)->value);
//             else if (v.obj->type == OBJ_PAIR) printf("Pair: %p\n", (void *)v.obj);
//             else printf("Obj(type=%d)\n", v.obj->type);
//         } else {
//             printf("<Invalid>\n");
//         }
//     }
//     if (empty) printf("(Memory is empty)\n");
//     printf("=================================\n");
// }

// void print_stack(Program *p) {
//     printf("\n=== VM HALTED ===\n");
//     if (p->sp == 0) {
//         printf("Stack is empty\n");
//         return;
//     }
//     printf("Stack (top -> bottom):\n");
//     for (int i = p->sp - 1; i >= 0; i--) {
//         Value v = p->stack[i];
//         if (v.type == VAL_OBJ && v.obj != NULL) {
//             if (v.obj->type == OBJ_INT) printf("[%d] ObjInt value=%d\n", i, ((ObjInt *)v.obj)->value);
//             else printf("[%d] Obj %p\n", i, (void *)v.obj);
//         } else {
//             printf("[%d] <invalid>\n", i);
//         }
//     }
// }

// int main(int argc, char **argv) {
//     if (argc < 2) {
//         fprintf(stderr, "usage: %s <bytecode_file> [memstat|gc|leaks]\n", argv[0]);
//         return 1;
//     }

//     const char *file = argv[1];
    
//     // 1. Detect Mode
//     int mode_memstat = (argc >= 3 && strcmp(argv[2], "memstat") == 0);
//     int mode_gc      = (argc >= 3 && strcmp(argv[2], "gc") == 0);
//     int mode_leaks   = (argc >= 3 && strcmp(argv[2], "leaks") == 0);

//     int size = 0;
//     unsigned char *code = load_bytecode(file, &size);
//     if (!code) return 1;

//     Program prog;
//     vm_init(&prog, code, size);

//     if (mode_memstat) {
//         // Lab 1: Profiler
//         printf("[MemStat] Profiling Memory...\n");
//         vm_run(&prog);
//         printf("--------------------------------------------------\n");
//         printf("Total Allocations: %d\n", total_objects_created);
//         printf("Total Freed:       %d\n", no_of_object_freed);
//         printf("Net Heap Size:     %d\n", total_objects_created - no_of_object_freed);
//         printf("--------------------------------------------------\n");

//     } else if (mode_gc) {
//         // Lab 5: Force GC
//         printf("[GC Command] Running Program...\n");
//         vm_run(&prog);
//         printf("[GC Command] Forcing Garbage Collection...\n");
//         int before_free = no_of_object_freed;
        
//         gc_collect(1); // Force debug output
        
//         int newly_freed = no_of_object_freed - before_free;
//         printf("--------------------------------------------------\n");
//         printf("GC Reclaimed: %d objects in this pass.\n", newly_freed);
//         printf("--------------------------------------------------\n");

//     } else if (mode_leaks) {
//         // Lab 5: Leaks
//         printf("[Leaks Command] Running Program...\n");
//         vm_run(&prog);
//         int live_objects = total_objects_created - no_of_object_freed;
//         printf("--------------------------------------------------\n");
//         if (live_objects == 0) printf("No memory leaks detected.\n");
//         else printf("WARNING: %d objects remain in heap.\n", live_objects);
//         printf("--------------------------------------------------\n");

//     } else {
//         // Default Run
//         vm_validate(&prog);
//         clock_t start = clock();
//         vm_run(&prog);
//         clock_t end = clock();
//         printf("Execution time: %f milliseconds\n", ((double)(end - start))* 1000.0 / CLOCKS_PER_SEC);
        
//         gc_collect(0); // Silent GC
//         print_stack(&prog);
//         print_memory(&prog);
//     }

//     vm_free(&prog);
//     return 0;
// }