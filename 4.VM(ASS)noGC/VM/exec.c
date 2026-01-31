#include "exec.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

static int read_int32(const unsigned char *code, int offset) {
    /* little-endian 4-byte integer */
    uint32_t b0 = (uint32_t)code[offset];
    uint32_t b1 = (uint32_t)code[offset + 1] << 8;
    uint32_t b2 = (uint32_t)code[offset + 2] << 16;
    uint32_t b3 = (uint32_t)code[offset + 3] << 24;
    return (int)(int32_t)(b0 | b1 | b2 | b3);
}

void vm_run(Program *p) {
   
    while (p->pc < p->code_size) {

        int pc = p->pc;
        unsigned char op = p->code[pc];

        //count instruction
        p->instr_count++;

        
        if (needs_operand(op))
            p->pc = pc + 5;
        else
            p->pc = pc + 1;

        switch (op) {
            case 0x01: { /* PUSH */
                int value = read_int32(p->code, pc + 1);
                vm_push(p, value);
                break;
            }
            case 0x02: /* POP */
                (void)vm_pop(p);
                break;
            case 0x03: { /* DUP */
                int value = vm_pop(p);
                vm_push(p, value);
                vm_push(p, value);
                break;
            }
            case 0x10: { /* ADD */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, a + b);
                break;
            }
            case 0x11: { /* SUB */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, a - b);
                break;
            }
            case 0x12: { /* MUL */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, a * b);
                break;
            }
            case 0x13: { /* DIV */
                int b = vm_pop(p);
                int a = vm_pop(p);
                if (b == 0) {
                    fprintf(stderr, "error: division by zero\n");
                    exit(1);
                }
                vm_push(p, a / b);
                break;
            }
            case 0x14: { /* EQ (==) */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, (a == b) ? 1 : 0);
                break;
            }
            case 0x15: { /* NEQ (!=) */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, (a != b) ? 1 : 0);
                break;
            }
            case 0x16: { /* LT (<) */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, (a < b) ? 1 : 0);
                break;
            }
            case 0x17: { /* GT (>) */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, (a > b) ? 1 : 0);
                break;
            }
            case 0x18: { /* LE (<=) */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, (a <= b) ? 1 : 0);
                break;
            }
            case 0x19: { /* GE (>=) */
                int b = vm_pop(p);
                int a = vm_pop(p);
                vm_push(p, (a >= b) ? 1 : 0);
                break;
            }
            case 0x20: { /* JMP */
                int addr = read_int32(p->code, pc + 1);
                if (addr < 0 || addr >= p->code_size) {
                    fprintf(stderr, "error: invalid jump address %d\n", addr);
                    exit(1);
                }
                p->pc = addr;
                break;
            }
            case 0x21: { /* JZ */
                int addr = read_int32(p->code, pc + 1);
                int value = vm_pop(p);
                if (value == 0) {
                    if (addr < 0 || addr >= p->code_size) {
                        fprintf(stderr, "error: invalid jump address %d\n", addr);
                        exit(1);
                    }
                    p->pc = addr;
                }
                break;
            }
            case 0x22: { /* JNZ */
                int addr = read_int32(p->code, pc + 1);
                int value = vm_pop(p);
                if (value != 0) {
                    if (addr < 0 || addr >= p->code_size) {
                        fprintf(stderr, "error: invalid jump address %d\n", addr);
                        exit(1);
                    }
                    p->pc = addr;
                }
                break;
            }
            case 0x30: { /* STORE */
                int idx = read_int32(p->code, pc + 1);
                int value = vm_pop(p);
                if (idx < 0 || idx >= MEM_SIZE) {
                    fprintf(stderr, "error: invalid memory index %d\n", idx);
                    exit(1);
                }
                p->memory[idx] = value;
                break;
            }
            case 0x31: { /* LOAD */
                int idx = read_int32(p->code, pc + 1);
                if (idx < 0 || idx >= MEM_SIZE) {
                    fprintf(stderr, "error: invalid memory index %d\n", idx);
                    exit(1);
                }
                vm_push(p, p->memory[idx]);
                break;
            }
            case 0x40: { /* CALL */
                int addr = read_int32(p->code, pc + 1);
                if (addr < 0 || addr >= p->code_size) {
                    fprintf(stderr, "error: invalid jump address %d\n", addr);
                    exit(1);
                }
                vm_push_ret(p, p->pc);
                p->pc = addr;
                break;
            }
            case 0x41: { /* RET */
                p->pc = vm_pop_ret(p);
                break;
            }
            case 0xFF: /* HALT */
                printf("Instruction count: %d\n", p->instr_count);
                return;
            default:
                fprintf(stderr, "error: invalid opcode 0x%x at pc=%d\n", op, pc);
                exit(1);
        }
    }
}
