#include "assembler.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_LABELS 50

/* label table */
char label_name[MAX_LABELS][32];
int  label_addr[MAX_LABELS];
int  label_count = 0;



/* check if word is a label */
int is_label(char *word) {
    int len = strlen(word);
    return (word[len - 1] == ':');
}

/* remove ':' from label */
void strip_colon(char *word) {
    word[strlen(word) - 1] = '\0';
}

/* save label with address */
void add_label(char *name, int addr) {
    strcpy(label_name[label_count], name);
    label_addr[label_count] = addr;
    label_count++;
}

/* find label index */
int find_label(char *name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(label_name[i], name) == 0)
            return i;
    }
    return -1;
}

/* opcode table */
int lookup_opcode(char *m, unsigned char *op) {
    if (strcmp(m, "PUSH") == 0) *op = 0x01;
    else if (strcmp(m, "POP") == 0) *op = 0x02;
    else if (strcmp(m, "DUP") == 0) *op = 0x03;

    else if (strcmp(m, "ADD") == 0) *op = 0x10;
    else if (strcmp(m, "SUB") == 0) *op = 0x11;
    else if (strcmp(m, "MUL") == 0) *op = 0x12;
    else if (strcmp(m, "DIV") == 0) *op = 0x13;
    else if (strcmp(m, "CMP") == 0) *op = 0x14;

    else if (strcmp(m, "JMP") == 0) *op = 0x20;
    else if (strcmp(m, "JZ")  == 0) *op = 0x21;
    else if (strcmp(m, "JNZ") == 0) *op = 0x22;

    else if (strcmp(m, "STORE") == 0) *op = 0x30;
    else if (strcmp(m, "LOAD")  == 0) *op = 0x31;

    else if (strcmp(m, "CALL") == 0) *op = 0x40;
    else if (strcmp(m, "RET")  == 0) *op = 0x41;

    else if (strcmp(m, "PAIR")  == 0) *op = 0x50;
    else if (strcmp(m, "LEFT")  == 0) *op = 0x51;
    else if (strcmp(m, "RIGHT") == 0) *op = 0x52;

    else if (strcmp(m, "HALT") == 0) *op = 0xFF;

    else return 0;

    return 1;
}


/* write 4-byte little endian integer */
void write_int32(FILE *out, int val) {
    fputc(val & 0xFF, out);
    fputc((val >> 8) & 0xFF, out);
    fputc((val >> 16) & 0xFF, out);
    fputc((val >> 24) & 0xFF, out);
}



// Detect labels and calculate byte offsets

void pass1_build_label_table(FILE *in) {
    char line[MAX_LINE];
    int pc = 0;

    while (fgets(line, MAX_LINE, in)) {

        /* remove comment */
        char *c = strchr(line, ';');
        if (c) *c = '\0';

        char *word = strtok(line, " \t\n");
        if (!word) continue;

        /* label detection */
        if (is_label(word)) {
            strip_colon(word);
            add_label(word, pc);
            continue;
        }

        /* instruction size */
        if (strcmp(word, "PUSH") == 0 || strcmp(word, "JMP") == 0 || strcmp(word, "JZ") == 0 || strcmp(word, "JNZ") == 0
           || strcmp(word, "CALL") == 0 || strcmp(word, "STORE") == 0 || strcmp(word, "LOAD") == 0)
            pc += 5;
        else
            pc += 1;
    }
}

//   Replace labels with addresses and emit bytecode 
void pass2_emit_bytecode(FILE *in, FILE *out) {
    char line[MAX_LINE];

    while (fgets(line, MAX_LINE, in)) {

        char *c = strchr(line, ';');
        if (c) *c = '\0';

        char *mnemonic = strtok(line, " \t\n");
        if (!mnemonic) continue;

        /* skip labels */
        if (is_label(mnemonic))
            continue;

        char *operand = strtok(NULL, " \t\n");

        unsigned char opcode;
        if (!lookup_opcode(mnemonic, &opcode)) {
            printf("error: unknown instruction '%s'\n", mnemonic);
            exit(1);
        }

        /* instructions that REQUIRE operand */
        if (
            strcmp(mnemonic, "PUSH")  == 0 ||
            strcmp(mnemonic, "JMP")   == 0 ||
            strcmp(mnemonic, "JZ")    == 0 ||
            strcmp(mnemonic, "JNZ")   == 0 ||
            strcmp(mnemonic, "STORE") == 0 ||
            strcmp(mnemonic, "LOAD")  == 0 ||
            strcmp(mnemonic, "CALL")  == 0
        ) {
            if (operand == NULL) {
                printf("error: missing operand for %s\n", mnemonic);
                exit(1);
            }
        }

        /* write opcode */
        fputc(opcode, out);

        /* PUSH val */
        if (strcmp(mnemonic, "PUSH") == 0) {
            write_int32(out, atoi(operand));
        }

        /* JMP / JZ / JNZ / CALL label */
        else if (
            strcmp(mnemonic, "JMP")  == 0 ||
            strcmp(mnemonic, "JZ")   == 0 ||
            strcmp(mnemonic, "JNZ")  == 0 ||
            strcmp(mnemonic, "CALL") == 0
        ) {
            int idx = find_label(operand);
            if (idx == -1) {
                printf("error: undefined label '%s'\n", operand);
                exit(1);
            }
            write_int32(out, label_addr[idx]);
        }

        /* STORE idx / LOAD idx */
        else if (
            strcmp(mnemonic, "STORE") == 0 ||
            strcmp(mnemonic, "LOAD")  == 0
        ) {
            write_int32(out, atoi(operand));
        }

        
    }
}

//   Assemble function
int assemble(char *infile, char *outfile) {
    FILE *in = fopen(infile, "r");
   

    if (!in ) {
        printf("file error\n");
        return 1;
    }

    clock_t start = clock();

    pass1_build_label_table(in);
    rewind(in);
    FILE *out = fopen(outfile, "wb");
    if (!out) {
        printf("file error\n");
        fclose(in);
        return 1;
    }
    pass2_emit_bytecode(in, out);

    clock_t end = clock();
    double time_taken = ((double)(end - start))* 1000.0 / CLOCKS_PER_SEC;
    printf("Assemble time: %f milliseconds\n", time_taken);
    
    int size = ftell(out);
    printf("Output bytecode size: %d bytes\n", size);
    

    fclose(in);
    fclose(out);
    return 0;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("use: %s input.asm output.bin\n", argv[0]);
        return 1;
    }
    return assemble(argv[1], argv[2]);
}
