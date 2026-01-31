#include "process_mgmt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h> // Required for basename() and dirname()
#include "../../3.lexor/src/lab_parser.h" // For run_parser

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

int create_process(char *filename){
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
    p->status = STATE_SUBMITTED; 
    p->exit_code = 0;

    // 3. Set Input Path
    strncpy(p->input_file, filename, 255);

    // 4. GENERATE OUTPUT FILENAME WITH PATH
    // Goal: "tests/valid/math.txt" -> "tests/valid/100_math.asm"

    // We make separate copies because dirname/basename can modify the string
    char path_copy_dir[1024];
    char path_copy_base[1024];
    
    // Ensure null termination
    strncpy(path_copy_dir, filename, sizeof(path_copy_dir) - 1);
    path_copy_dir[sizeof(path_copy_dir) - 1] = '\0';
    
    strncpy(path_copy_base, filename, sizeof(path_copy_base) - 1);
    path_copy_base[sizeof(path_copy_base) - 1] = '\0';

    char *dir = dirname(path_copy_dir);
    char *base = basename(path_copy_base);

    // Format: "Directory / PID _ BaseName"
    // Example: "tests/valid/100_test.txt"
    snprintf(p->output_file, 255, "%s/%d_%s", dir, p->pid, base);

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



//code --- PS COMMAND HANDLER ---
int handle_ps(char **args) {
    if (strcmp(args[0], "ps") != 0) return 0;
    print_process_list(); // Defined in process_mgmt.c
    return 1;
}

// --- SUBMIT COMMAND HANDLER ---
int handle_submit(char **args) {
    if (strcmp(args[0], "submit") != 0) return 0;

    if (args[1] == NULL) {
        printf("Usage: submit <filename>\n");
        return 1;
    }

    // 1. Resolve absolute path (Good practice!)
    char full_input_path[4096];
    if (realpath(args[1], full_input_path) == NULL) {
        perror("Error finding file");
        return 1;
    }

    // 2. Create Process Entry
    // We pass the simple name to the process table for display purposes
    int pid = create_process(args[1]); 
    if (pid == -1) {
        printf("[Failed] Process table full.\n");
        return 1;
    }

    Process *proc = get_process(pid);
    printf("Process created with PID: %d\n", pid);
    printf("Submitting %s to parser...\n", full_input_path);

    // 3. CALL THE PARSER
    // run_parser generates "output.asm" in the current directory
    // We pass 0 for do_eval (compile only)
    int result = run_parser(full_input_path, 0); 

    if (result == 0) {
        // --- SUCCESS LOGIC ---
        
        // A. Rename the generic output to the specific output file
        // (e.g., rename "output.asm" to "test.asm")
        if (rename("output.asm", proc->output_file) != 0) {
            perror("[Warning] Could not rename output.asm");
            // If rename fails, we still mark it submitted but warn the user
        }

        // B. Update Process State
        update_process_state(pid, STATE_SUBMITTED);
        
        printf("[Success] AST Generated & ASM Written to '%s'.\n", proc->output_file);
        printf("Ready to run. Type: run %d\n", pid);

    } else {
        // --- FAILURE LOGIC ---
        update_process_state(pid, STATE_FAILED);
        printf("[Failed] Syntax errors found. Process marked FAILED.\n");
    }

    return 1;
}

// --- RUN COMMAND HANDLER ---
int handle_run(char **args) {
    if (strcmp(args[0], "run") != 0) return 0;

    if (args[1] == NULL) {
        printf("Usage: run <PID>\n");
        return 1;
    }

    // 1. Look up process
    int pid = atoi(args[1]);
    Process *proc = get_process(pid);

    if (!proc) {
        printf("[Error] PID %d not found.\n", pid);
        return 1;
    }

    if (proc->status != STATE_SUBMITTED && proc->status != STATE_TERMINATED) {
        printf("[Error] Process not ready. Status: %d. (Did you submit it?)\n", proc->status);
        return 1;
    }

    // 2. Define Executable Paths (Relative to mini-shell folder)
    // NOTE: We use single quotes '' around paths to handle the parentheses in "(ASS)"
    const char *ASSEMBLER_BIN = "../5.VM(ASS)withGC/assembler";
    const char *VM_BIN        = "../5.VM(ASS)withGC/bvm";

    // 3. Construct Bytecode Filename (test.asm -> test.byc)
    char bytecode_file[256];
    strcpy(bytecode_file, proc->output_file);
    char *dot = strrchr(bytecode_file, '.');
    if (dot) {
        strcpy(dot, ".byc");
    } else {
        strcat(bytecode_file, ".byc");
    }

    // 4. STEP A: Run Assembler
    // Command: "../5.VM.../assembler" "test.asm" "test.byc"
    char cmd[1024];
    printf("[Shell] Assembling '%s' -> '%s'...\n", proc->output_file, bytecode_file);
    
    // We quote the executable path to handle special characters like '(' in directory names
    snprintf(cmd, sizeof(cmd), "'%s' %s %s", ASSEMBLER_BIN, proc->output_file, bytecode_file);
    
    int asm_ret = system(cmd);
    if (asm_ret != 0) {
        printf("[Shell] Assembly Failed. (Check if assembler path is correct)\n");
        update_process_state(pid, STATE_FAILED);
        return 1;
    }

    // 5. STEP B: Run VM
    // Command: "../5.VM.../bvm" "test.byc"
    printf("[Shell] Starting VM for PID %d...\n", pid);
    printf("--------------------------------------------------\n");
    
    update_process_state(pid, STATE_RUNNING);
    
    snprintf(cmd, sizeof(cmd), "'%s' %s", VM_BIN, bytecode_file);
    int vm_ret = system(cmd);
    
    printf("--------------------------------------------------\n");

    if (vm_ret == 0) {
        printf("[Shell] Process %d Finished Successfully.\n", pid);
        update_process_state(pid, STATE_TERMINATED);
    } else {
        printf("[Shell] Process %d Crashed or Returned Error.\n", pid);
        update_process_state(pid, STATE_FAILED);
    }

    return 1;
}

// --- KILL COMMAND HANDLER ---
int handle_kill(char **args) {
    if (strcmp(args[0], "kill") != 0) return 0;

    if (args[1] == NULL) {
        printf("Usage: kill <PID>\n");
        return 1;
    }

    int pid = atoi(args[1]);
    Process *proc = get_process(pid);

    if (!proc) {
        printf("[Error] PID %d not found.\n", pid);
        return 1;
    }

    // 1. Delete .asm file
    if (remove(proc->output_file) == 0) {
        printf("[Kill] Deleted %s\n", proc->output_file);
    }

    // 2. Delete .byc file
    char bytecode_file[256];
    strcpy(bytecode_file, proc->output_file);
    char *dot = strrchr(bytecode_file, '.');
    if (dot) strcpy(dot, ".byc");
    else strcat(bytecode_file, ".byc");

    if (remove(bytecode_file) == 0) {
        printf("[Kill] Deleted %s\n", bytecode_file);
    }

    // 3. Free Process Table Entry
    delete_process(pid);
    printf("[Kill] Process %d terminated and removed from table.\n", pid);

    return 1;
}

