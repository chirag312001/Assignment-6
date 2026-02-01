#include "process_mgmt.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <libgen.h> // Required for basename() and dirname()

#include "../../3.lexor/src/lab_parser.h" // For run_parser
#include "../../5.VM(ASS)withGC/debugger/debugger.h"
#include "../../5.VM(ASS)withGC/VM/loader.h"

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

    snprintf(p->bytecode_file, 255, "%s/%d_%s", dir, p->pid, base);
    dot = strrchr(p->bytecode_file, '.');
    if (dot) {
        strcpy(dot, ".byc");
    } else {
        strcat(p->bytecode_file, ".byc");
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
    printf("\n%-5s %-12s %-18s %-18s %-18s\n", "PID", "STATUS", "INPUT", "ASM", "BYC");
    printf("--------------------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status != STATE_NONE) {
            const char *s = "UNKNOWN";
            if(process_table[i].status == STATE_SUBMITTED) s = "SUBMITTED";
            else if(process_table[i].status == STATE_RUNNING) s = "RUNNING";
            else if(process_table[i].status == STATE_TERMINATED) s = "FINISHED";
            else if(process_table[i].status == STATE_FAILED) s = "FAILED";
            
            printf("%-5d %-12s %-18s %-18s %-18s\n", 
                process_table[i].pid, s, 
                process_table[i].input_file, 
                process_table[i].output_file,
                process_table[i].bytecode_file);
        }
    }
}



//signal handler
// zommbie process clean up handler, WNOHANG is wait no hang
void handle_zoombi(){
    int pid;
    
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0){
        
    }
    
}

void sigint_handle(int sig) {

    write(STDOUT_FILENO, "\n", 1);

    char curr_working_dir[4000];
    getCurrDir(curr_working_dir, sizeof(curr_working_dir));


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

    // Check status (Allowing SUBMITTED, TERMINATED, or FAILED for a retry)
    if (proc->status == STATE_NONE || proc->status == STATE_RUNNING) {
        printf("[Error] Process not ready. Status: %d.\n", proc->status);
        return 1;
    }

    // 2. Define Executable Paths
    const char *ASSEMBLER_BIN = "../5.VM(ASS)withGC/assembler";
    const char *VM_BIN        = "../5.VM(ASS)withGC/bvm";

    char cmd[1024];

    // 3. STEP A: Run Assembler
    // Input: proc->output_file (.asm) | Output: proc->bytecode_file (.byc)
    printf("[Shell] Assembling '%s' -> '%s'...\n", proc->output_file, proc->bytecode_file);
    
    // Use single quotes around paths to handle the () in the folder name
    snprintf(cmd, sizeof(cmd), "'%s' '%s' '%s'", 
             ASSEMBLER_BIN, proc->output_file, proc->bytecode_file);
    
    int asm_ret = system(cmd);
    if (asm_ret != 0) {
        printf("[Shell] Assembly Failed.\n");
        update_process_state(pid, STATE_FAILED);
        return 1;
    }

    // 4. STEP B: Run VM
    printf("[Shell] Starting VM for PID %d...\n", pid);
    printf("--------------------------------------------------\n");
    
    update_process_state(pid, STATE_RUNNING);
    
    // Pass the .byc file to the VM
    snprintf(cmd, sizeof(cmd), "'%s' '%s'", VM_BIN, proc->bytecode_file);
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



// debugger integration 
void debug_program(int pid) {
    Process *proc = get_process(pid);
    if (!proc) {
        printf("Shell Error: PID %d not found.\n", pid);
        return;
    }

    pid_t child_pid = fork();

    if (child_pid == 0) {
        /* --- CHILD PROCESS (The Debugger) --- */
        signal(SIGINT, SIG_DFL);
        // signal(SIGQUIT, SIG_DFL);

        // 1. Run the Assembler before starting the debugger
        // We now use both proc->output_file (.asm) and proc->bytecode_file (.byc)
        char assemble_cmd[1024];
        sprintf(assemble_cmd, "'../5.VM(ASS)withGC/assembler' '%s' '%s'", 
                proc->output_file, proc->bytecode_file);

        printf("[Debugger] Syncing: Assembling %s -> %s...\n", 
                proc->output_file, proc->bytecode_file);

        // Execute assembly. system() returns 0 on success.
        if (system(assemble_cmd) != 0) {
            fprintf(stderr, "Debugger Error: Assembly failed. Check if path/file exists.\n");
            exit(1);
        }

        // 2. Load the freshly generated bytecode directly from the struct
        int size = 0;
        unsigned char *code = load_bytecode(proc->bytecode_file, &size);
        if (!code) {
            fprintf(stderr, "Debugger Error: Could not load bytecode file %s\n", proc->bytecode_file);
            exit(1);
        }

        // 3. Initialize VM and start
        Program prog;
        vm_init(&prog, code, size);
        debug_start(&prog);

        vm_free(&prog);
        exit(0); 

    } else if (child_pid > 0) {
        /* --- PARENT PROCESS (The Shell) --- */
        signal(SIGINT, SIG_IGN);
        waitpid(child_pid, NULL, 0);
        signal(SIGCHLD, handle_zoombi);
        signal(SIGINT, sigint_handle); 

        printf("[Shell] Debug session for PID %d ended.\n", pid);
    } else {
        perror("Fork failed");
    }
}