#include "process_mgmt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h> // Required for basename()

Process process_table[MAX_PROCESSES];
int next_pid = 100; // Start PIDs at 100

void delete_process(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            process_table[i].status = STATE_NONE;
            process_table[i].pid = -1;
            process_table[i].input_file[0] = '\0';
            process_table[i].output_file[0] = '\0';
            return;
        }
    }
}

void init_process_table() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].status = STATE_NONE;
        process_table[i].pid = -1;
    }
}

// NOTE: This helper is no longer used by create_process but kept if needed elsewhere.
void generate_output_name(const char *input, char *output) {
    strcpy(output, input);
    char *dot = strrchr(output, '.');
    if (dot) {
        strcpy(dot, ".asm");
    } else {
        strcat(output, ".asm");
    }
}

int create_process(char *filename) {
    // 1. Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == STATE_NONE) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("Error: Process table full.\n");
        return -1;
    }

    // 2. Initialize Process
    Process *p = &process_table[slot];
    p->pid = next_pid++;
    p->status = STATE_SUBMITTED; // Changed default to SUBMITTED (waiting for compile)
    p->exit_code = 0;

    // 3. Set Input Path
    strncpy(p->input_file, filename, 255);

    // 4. GENERATE UNIQUE OUTPUT FILENAME
    // Strategy: PID_Basename.asm (e.g., 100_var_decl.asm)
    
    // We create a temp copy because basename() can modify the string passed to it
    char temp_path[256];
    strncpy(temp_path, filename, 255);
    char *base = basename(temp_path);

    // Prepend PID to the filename
    snprintf(p->output_file, 255, "%d_%s", p->pid, base);

    // Change extension to .asm
    char *dot = strrchr(p->output_file, '.');
    if (dot) {
        strcpy(dot, ".asm");
    } else {
        strcat(p->output_file, ".asm");
    }

    return p->pid;
}

Process* get_process(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            return &process_table[i];
        }
    }
    return NULL;
}

void update_process_state(int pid, ProcessStatus new_state) {
    Process *p = get_process(pid);
    if (p) {
        p->status = new_state;
    }
}

void print_process_list() {
    printf("\n%-5s %-15s %-20s %-20s\n", "PID", "STATUS", "INPUT", "OUTPUT");
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status != STATE_NONE) {
            const char *status_str = "UNKNOWN";
            switch(process_table[i].status) {
                case STATE_SUBMITTED: status_str = "SUBMITTED"; break;
                case STATE_RUNNING:   status_str = "RUNNING"; break;
                case STATE_PAUSED:    status_str = "PAUSED"; break;
                case STATE_TERMINATED:status_str = "TERMINATED"; break;
                case STATE_FAILED:    status_str = "FAILED"; break;
                default: break;
            }
            printf("%-5d %-15s %-20s %-20s\n", 
                process_table[i].pid, status_str, 
                process_table[i].input_file, process_table[i].output_file);
        }
    }
    printf("\n");
}