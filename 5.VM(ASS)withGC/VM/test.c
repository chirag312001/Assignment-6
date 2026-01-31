#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "vm.h"
#include "stack.h"
#include <time.h>

void run_with_timer(const char *name, void (*test_fn)(Program*), Program *p) {
    clock_t start = clock();
    test_fn(p);
    clock_t end = clock();

    double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Time taken [%s]: %.3f ms\n", name, ms);
}


/* Helper to simulate the VM state for GC tests */
void setup_test_vm(Program* p) {
    p->sp = 0;
    p->csp = 0;
    p->instr_count = 0;
    for (int i = 0; i < MEM_SIZE; i++) {
        p->memory[i].type = VAL_NIL;
        p->memory[i].obj = NULL;
    }
    current_program = p;
}


// 1.6.1 & 1.6.2: Basic Reachability & Unreachable Object Collection
void test_reachability(Program* p) {
    printf("\nRunning: Basic Reachability & Unreachable Collection\n");
    ObjPair* a = new_pair(make_obj(NULL), make_obj(NULL));
    vm_push(p, make_obj((Obj*)a));
    
    no_of_object_freed = 0;
    gc_collect();
    printf("\nGC SUMMARY when stack is not empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
    no_of_object_freed = 0;
    vm_pop(p);
   
    gc_collect();
    printf("\nGC SUMMARY when stack is empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
}

// 1.6.3: Transitive Reachability
void test_transitive(Program* p) {
    printf("\nRunning: Transitive Reachability\n");
    ObjPair* a = new_pair(make_obj(NULL), make_obj(NULL));
    ObjPair* b = new_pair(make_obj((Obj*)a), make_obj(NULL));
    vm_push(p, make_obj((Obj*)b));
    
    no_of_object_freed = 0;
    gc_collect();
    printf("\nGC SUMMARY when stack is not empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
    no_of_object_freed = 0;
    vm_pop(p);
   
    gc_collect();
    printf("\nGC SUMMARY when stack is empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
}

// 1.6.4: Cyclic References
void test_cycles(Program* p) {
    printf("\nRunning: Cyclic References\n");
    ObjPair* a = new_pair(make_obj(NULL), make_obj(NULL));
    ObjPair* b = new_pair(make_obj((Obj*)a), make_obj(NULL));
    a->right = make_obj((Obj*)b); // Create cycle: A <-> B
    
    vm_push(p, make_obj((Obj*)a));
   
    gc_collect();
    printf("\nGC SUMMARY when stack is not empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
    no_of_object_freed = 0;
    vm_pop(p);
   
    gc_collect();
    printf("\nGC SUMMARY when stack is empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
}

// 1.6.5: Deep Object Graph (Stress Test)
void test_deep_graph(Program* p) {
    printf("\nRunning: Deep Object Graph Stress Test\n");
    ObjPair* root = new_pair(make_obj(NULL), make_obj(NULL));
    ObjPair* cur = root;
    
    // Create a long chain of objects
    for (int i = 0; i < 500; i++) {
        ObjPair* next = new_pair(make_obj(NULL), make_obj(NULL));
        cur->right = make_obj((Obj*)next);
        cur = next;
    }
    
    vm_push(p, make_obj((Obj*)root));
    
    no_of_object_freed = 0;
    gc_collect();
    printf("\nGC SUMMARY when stack is not empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
    no_of_object_freed = 0;
    vm_pop(p);
    
    gc_collect();
    printf("\nGC SUMMARY when stack is empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
}

// 1.6.6: Closure Capture
void test_closures(Program* p) {
    printf("\nRunning: Closure Capture\n");
    Obj* env = (Obj*)new_pair(make_int(10), make_int(20));
    Obj* fn = new_function();
    Obj* cl = new_closure(fn, env);
    
    vm_push(p, make_obj(cl));
    
    no_of_object_freed = 0;
    gc_collect();
    printf("\nGC SUMMARY when stack is not empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
    no_of_object_freed = 0;
    vm_pop(p);
   
    gc_collect();
    printf("\nGC SUMMARY when stack is empty\n");
    printf("Objects freed: %d\n", no_of_object_freed);
}

// 1.6.7: Stress Allocation
void test_stress_allocation(Program* p) {
    printf("\nRunning: Stress Allocation\n");
    for (int i = 0; i < 1000; i++) {
        new_pair(make_obj(NULL), make_obj(NULL));
    }

    /* No roots: all objects should be collected */
    no_of_object_freed = 0;
    gc_collect();
    printf("\nGC SUMMARY\n");
    printf("Objects freed: %d\n", no_of_object_freed);
}



int main() {
    Program prog;
    setup_test_vm(&prog);
    
    int choice;
    while (1) {
        printf("\n--- GC Interactive Test Suite ---\n");
        printf("1. Basic Reachability (1.6.1 & 1.6.2)\n");
        printf("2. Transitive Reachability (1.6.3)\n");
        printf("3. Cyclic References (1.6.4)\n");
        printf("4. Deep Object Graph (1.6.5)\n");
        printf("5. Closure Capture (1.6.6)\n");
        printf("6. Stress Allocation (1.6.7)\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        
        if (scanf("%d", &choice) != 1) break;
        if (choice == 0) break;
        switch (choice) {
            case 1:
                run_with_timer("Basic Reachability", test_reachability, &prog);
                break;
            case 2:
                run_with_timer("Transitive Reachability", test_transitive, &prog);
                break;
            case 3:
                run_with_timer("Cyclic References", test_cycles, &prog);
                break;
            case 4:
                run_with_timer("Deep Object Graph", test_deep_graph, &prog);
                break;
            case 5:
                run_with_timer("Closure Capture", test_closures, &prog);
                break;
            case 6:
                run_with_timer("Stress Allocation", test_stress_allocation, &prog);
                break;
        }
        
        // Final safety cleanup after each test run
        if (heap_objects != NULL) {
            printf("\nCleaning up remaining heap objects...\n");
            gc_collect();
        }
    }

    return 0;
}
