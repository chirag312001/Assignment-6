// include/process_mgmt.h
#ifndef PROCESS_MGMT_H
#define PROCESS_MGMT_H

#define MAX_PROCESSES 100

typedef enum {
    STATE_NONE,
    STATE_SUBMITTED, // Code parsed & ASM generated
    STATE_RUNNING,   // Currently executing in VM
    STATE_PAUSED,
    STATE_TERMINATED,
    STATE_FAILED
} ProcessStatus;

typedef struct {
    int pid;                // Unique Program ID
    char input_file[256];   // Original source (.lang)
    char output_file[256];  // Generated assembly (.asm)
    ProcessStatus status;   // Current state
    int exit_code;          // Exit code
} Process;

// Global process table
extern Process process_table[MAX_PROCESSES];
extern int next_pid;

// Function prototypes
void init_process_table();
int create_process(char *input_filename);
Process* get_process(int pid);
void update_process_state(int pid, ProcessStatus new_state);
void print_process_list();

#endif