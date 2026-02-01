#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debugger.h"
#include "../VM/exec.h"

#define MAX_BPS 32

typedef struct {
    int addr;
    int active;
} Breakpoint;

static Breakpoint bp_table[MAX_BPS];
static int bp_count = 0;

/* --- BP Management Functions --- */

void add_breakpoint(int addr) {
    for (int i = 0; i < MAX_BPS; i++) {
        if (bp_table[i].active && bp_table[i].addr == addr) {
            printf("Breakpoint already exists at %d\n", addr);
            return;
        }
    }
    for (int i = 0; i < MAX_BPS; i++) {
        if (!bp_table[i].active) {
            bp_table[i].addr = addr;
            bp_table[i].active = 1;
            printf("Breakpoint %d set at address %d\n", i + 1, addr);
            bp_count++;
            return;
            
        }
    }
}

void delete_breakpoint(int n) {
    if (n > 0 && n <= MAX_BPS && bp_table[n-1].active) {
        bp_table[n-1].active = 0;
        printf("Breakpoint %d deleted.\n", n);
    } else {
        printf("Invalid breakpoint ID.\n");
    }
}

void clear_breakpoints() {
    for (int i = 0; i < MAX_BPS; i++) bp_table[i].active = 0;
    printf("All breakpoints cleared.\n");
}

int is_breakpoint(int pc) {
    for (int i = 0; i < MAX_BPS; i++) {
        if (bp_table[i].active && bp_table[i].addr == pc) return 1;
    }
    return 0;
}

/* --- Disassembler (List) --- */

void list_code(Program *p) {
    printf("\n--- Disassembly (Next 10 Instructions) ---\n");
    int cur = p->pc;
    
    // We will show 10 instructions or until we hit the end of the bytecode
    for (int i = 0; i < 10 && cur < p->code_size; i++) {
        unsigned char op = p->code[cur];
        
        // Print the indicator for current PC and the address
        printf(" %s %04d: ", (cur == p->pc) ? "->" : "  ", cur);

        switch (op) {
            /* --- 5-Byte Instructions (Opcode + Int32) --- */
            case 0x01: printf("PUSH   %d\n", *((int*)&p->code[cur+1])); cur += 5; break;
            case 0x20: printf("JMP    %d\n", *((int*)&p->code[cur+1])); cur += 5; break;
            case 0x21: printf("JZ     %d\n", *((int*)&p->code[cur+1])); cur += 5; break;
            case 0x22: printf("JNZ    %d\n", *((int*)&p->code[cur+1])); cur += 5; break;
            case 0x30: printf("STORE  %d\n", *((int*)&p->code[cur+1])); cur += 5; break;
            case 0x31: printf("LOAD   %d\n", *((int*)&p->code[cur+1])); cur += 5; break;
            case 0x40: printf("CALL   %d\n", *((int*)&p->code[cur+1])); cur += 5; break;

            /* --- 1-Byte Instructions (Arithmetic) --- */
            case 0x02: printf("POP\n");    cur += 1; break;
            case 0x03: printf("DUP\n");    cur += 1; break;
            case 0x10: printf("ADD\n");    cur += 1; break;
            case 0x11: printf("SUB\n");    cur += 1; break;
            case 0x12: printf("MUL\n");    cur += 1; break;
            case 0x13: printf("DIV\n");    cur += 1; break;
            case 0x14: printf("EQ\n");     cur += 1; break;
            case 0x15: printf("NEQ\n");    cur += 1; break;
            case 0x16: printf("LT\n");     cur += 1; break;
            case 0x17: printf("GT\n");     cur += 1; break;
            case 0x18: printf("LE\n");     cur += 1; break;
            case 0x19: printf("GE\n");     cur += 1; break;

            /* --- 1-Byte Instructions (Control/Objects) --- */
            case 0x41: printf("RET\n");    cur += 1; break;
            case 0x50: printf("PAIR\n");   cur += 1; break;
            case 0x51: printf("LEFT\n");   cur += 1; break;
            case 0x52: printf("RIGHT\n");  cur += 1; break;
            
            case 0xFF: 
                printf("HALT\n");   
                cur = p->code_size; // Stop listing after HALT
                break;

            default:
                printf("UNKNOWN (0x%02X)\n", op);
                cur += 1;
                break;
        }
    }
    printf("------------------------------------------\n");
}

/* --- Main Debug Loop --- */

void debug_start(Program *p) {
    char cmd[64];
    int val;

    printf("\n=== FULLY-FLEDGED VM DEBUGGER ===\n");
    printf("Commands: step, continue, break <addr>, delete <id>, info break, clear, list, exit\n");

    while (1) {
        printf("(debug pc=%d) > ", p->pc);
        if (scanf("%63s", cmd) <= 0) break;

        if (strcmp(cmd, "step") == 0) {
            if (!vm_step(p)) {
                printf("\n[System] Execution Finished (HALT reached at PC %d).\n", p->pc);
                break; // Exit debugger loop
            }
        } 
        else if (strcmp(cmd, "continue") == 0) {
            // printf("Resuming execution...\n");

            // Step once to move past current breakpoint if we are sitting on one
            if (is_breakpoint(p->pc)) {
                if (!vm_step(p)) {
                    printf("\n[System] Execution Finished.\n");
                    return;
                }
            }

            int running = 1;
            while (p->pc < p->code_size && !is_breakpoint(p->pc)) {
                if (!vm_step(p)) {
                    printf("\n[System] Execution Finished (HALT reached at PC %d).\n", p->pc);
                    running = 0;
                    break;
                }
            }

            if (!running) {
                // Program finished during the continue loop
                return; 
            }

            if (is_breakpoint(p->pc)) {
                printf("\n[TRAP] Hit breakpoint at address %d\n", p->pc);
            }
        }
        // else if(strcmp(cmd, "BP_count") == 0) {

        //     printf("Total Breakpoints: %d\n", bp_count);

        // }
        else if (strcmp(cmd, "break") == 0) {

            if (scanf("%d", &val)) add_breakpoint(val);

        }else if (strcmp(cmd, "delete") == 0) {

            if (scanf("%d", &val)) delete_breakpoint(val);

        }else if (strcmp(cmd, "clear") == 0) {

            clear_breakpoints();
            bp_count = 0;
        }else if (strcmp(cmd, "info") == 0) {

            scanf("%63s", cmd); // check for "break"
            for(int i=0; i<MAX_BPS; i++) 
                if(bp_table[i].active) printf("[%d] Address: %d\n", i+1, bp_table[i].addr);
            printf("Total Breakpoints: %d\n", bp_count);

        }else if (strcmp(cmd, "list") == 0) {

            list_code(p);

        }else if (strcmp(cmd, "exit") == 0){
            printf("Exiting debugger.\n");
            break;
        }
        else if (strcmp(cmd, "memstat") == 0) {
            // These externs should point to your GC variables in vm.c or gc.c
            extern int total_objects_created;
            extern int no_of_object_freed;
            printf("--- Heap Report ---\n");
            printf("Objects Created: %d\n", total_objects_created);
            printf("Objects Freed:   %d\n", no_of_object_freed);
            printf("Live on Heap:    %d\n", total_objects_created - no_of_object_freed);
        } 
        else if (strcmp(cmd, "gc") == 0) {
            printf("Triggering Garbage Collection...\n");
            // gc_collect(prog); // Pass your program/VM state to the GC
        }
        else if (strcmp(cmd, "leaks") == 0) {
            // Logic to check if anything is reachable that shouldn't be
            // Usually a simple count of live objects at the end of execution
            extern int total_objects_created;
            extern int no_of_object_freed;
            printf("Potential Leaks: %d objects\n", total_objects_created - no_of_object_freed);
        }
        else {
            printf("Unknown command: %s\n", cmd);
        }
    }
}