#include "loader.h"

#include <stdio.h>
#include <stdlib.h>

static int valid_opcode(unsigned char op) {
    /* whitelist of supported opcodes */
    switch (op) {
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x30:
        case 0x31:
        case 0x40:
        case 0x41:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0xFF:
            return 1;
        default:
            return 0;
    }
}

static int needs_operand(unsigned char op) {
    /* instructions that carry a 4-byte operand */
    return (
        op == 0x01 ||  /* PUSH */
        op == 0x20 ||  /* JMP */
        op == 0x21 ||  /* JZ */
        op == 0x22 ||  /* JNZ */
        op == 0x30 ||  /* STORE */
        op == 0x31 ||  /* LOAD */
        op == 0x40     /* CALL */
    );
}

unsigned char *load_bytecode(const char *file, int *size) {
    FILE *f = fopen(file, "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", file);
        return NULL;
    }

    /* read full file into memory */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buf = malloc(file_size);
    if (!buf) {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, file_size, f) != (size_t)file_size) {
        fprintf(stderr, "error: truncated bytecode\n");
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);

    *size = (int)file_size;
    return buf;
}

int vm_validate(Program *p) {
    int pc = 0;

    while (pc < p->code_size) {

        unsigned char op = p->code[pc++];

        /* opcode check */
        if (!valid_opcode(op)) {
            fprintf(stderr, "error: invalid opcode 0x%x at pc=%d\n", op, pc - 1);
            exit(1);
        }

        /* truncation check */
        if (needs_operand(op)) {
            if (pc + 4 > p->code_size) {
                fprintf(stderr, "error: truncated instruction at pc=%d\n", pc - 1);
                exit(1);
            }
            pc += 4;  /* skip operand */
        }

        /* HALT stops program */
        if (op == 0xFF) {
            return 1;  /* valid bytecode */
        }
    }

    /* if no HALT found */
    fprintf(stderr, "error: program has no HALT instruction\n");
    exit(1);
}
