#include "exec.h"
#include "stack.h"
#include "include/object.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Pop a Value and enforce int type for arithmetic/control ops. */
// static int pop_int(Program *p) {
//     Value v = vm_pop(p);
//     if (v.type != VAL_INT) {
//         fprintf(stderr, "error: expected int on stack\n");
//         exit(1);
//     }
//     return v.int_val;
// }
static int pop_int(Program *p) {
    Value v = vm_pop(p);

    if (v.type != VAL_OBJ || v.obj == NULL || v.obj->type != OBJ_INT) {
        fprintf(stderr, "error: expected integer object on stack\n");
        exit(1);
    }

    return ((ObjInt *)v.obj)->value;
}

/* Push an int as a Value to keep stack/memory uniform. */
static void push_int(Program *p, int value) {
    vm_push(p, make_int(value));
}

/* Pop and validate a pair object. */
static ObjPair *pop_pair(Program *p) {
    Value v = vm_pop(p);
    if (v.type != VAL_OBJ || v.obj == NULL || v.obj->type != OBJ_PAIR) {
        fprintf(stderr, "error: expected pair object on stack\n");
        exit(1);
    }
    return (ObjPair *)v.obj;
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
                push_int(p, value);
                break;
            }
            case 0x02: /* POP */
                (void)vm_pop(p);
                break;
            case 0x03: { /* DUP */
                Value value = vm_pop(p);
                vm_push(p, value);
                vm_push(p, value);
                break;
            }
            case 0x10: { /* ADD */
                int b = pop_int(p);
                int a = pop_int(p);
                push_int(p, a + b);
                break;
            }
            case 0x11: { /* SUB */
                int b = pop_int(p);
                int a = pop_int(p);
                push_int(p, a - b);
                break;
            }
            case 0x12: { /* MUL */
                int b = pop_int(p);
                int a = pop_int(p);
                push_int(p, a * b);
                break;
            }
            case 0x13: { /* DIV */
                int b = pop_int(p);
                int a = pop_int(p);
                if (b == 0) {
                    fprintf(stderr, "error: division by zero\n");
                    exit(1);
                }
                push_int(p, a / b);
                break;
            }
            case 0x14: { /* CMP */
                int b = pop_int(p);
                int a = pop_int(p);
                push_int(p, (a < b) ? 1 : 0);
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
                int value = pop_int(p);
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
                int value = pop_int(p);
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
                Value value = vm_pop(p);
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
            case 0x50: { /* PAIR */
                Value right = vm_pop(p);
                Value left = vm_pop(p);
                ObjPair *pair = new_pair(left, right);
                if (!pair) {
                    fprintf(stderr, "error: out of memory\n");
                    exit(1);
                }
                vm_push(p, make_obj((Obj *)pair));
                break;
            }
            case 0x51: { /* LEFT */
                ObjPair *pair = pop_pair(p);
                vm_push(p, pair->left);
                break;
            }
            case 0x52: { /* RIGHT */
                ObjPair *pair = pop_pair(p);
                vm_push(p, pair->right);
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
