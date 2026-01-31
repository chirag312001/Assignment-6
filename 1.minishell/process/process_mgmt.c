// include/process_mgmt.c
#include "process_mgmt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Process process_table[MAX_PROCESSES];
int next_pid = 100; // Start PIDs at 100

void init_process_table() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].status = STATE_NONE;
        process_table[i].pid = -1;
    }
}

// Helper to replace extension (test.lang -> test.asm)
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
    // Find free slot
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

    // Initialize Process
    Process *p = &process_table[slot];
    p->pid = next_pid++;
    p->status = STATE_FAILED; // Default until compilation succeeds
    p->exit_code = 0;

    // Set paths
    strncpy(p->input_file, filename, 255);
    generate_output_name(filename, p->output_file);

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