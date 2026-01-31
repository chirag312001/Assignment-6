// 3.lexor/src/ir.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
#include "ast.h"
#include "../build/parser.tab.h"

// --- Helper: Symbol Table for Assembly ---
// Maps variable names (char*) to memory slots (int)
typedef struct {
    char *name;
    int index;
} VarMap;

static VarMap variables[100];
static int var_count = 0;
static int label_counter = 0;
static FILE *out_file = NULL;

// Find or create a slot index for a variable
static int get_var_index(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].index;
        }
    }
    // New variable, assign next slot
    variables[var_count].name = strdup(name);
    variables[var_count].index = var_count;
    return var_count++;
}

static int new_label() {
    return ++label_counter;
}

// --- Helper: Emit Instructions ---
static void emit(const char *instr, int val, const char *comment) {
    
    // Check if this is a Jump instruction that needs a Label
    if (strcmp(instr, "JMP") == 0 || strcmp(instr, "JZ") == 0 || strcmp(instr, "JNZ") == 0) {
        // Fix: Print "JZ L001" instead of "JZ 1"
        fprintf(out_file, "%s L%03d", instr, val);
    } 
    else if (val == -1) {
        // Instructions with no operand (ADD, SUB, HALT)
        fprintf(out_file, "%s", instr);
    } 
    else {
        // Standard instructions (PUSH 5, STORE 0)
        fprintf(out_file, "%s %d", instr, val);
    }
    
    if (comment) 
        fprintf(out_file, "\t\t; %s", comment);
    
    fprintf(out_file, "\n");
}

static void emit_label(int label_id) {
    fprintf(out_file, "L%03d:\n", label_id);
}

// --- Recursive Traversal ---
static void gen(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            // Loop through all statements in the program
            for (ASTNodeList *cur = node->as.program.statements; cur; cur = cur->next) {
                gen(cur->node);
            }
            break;

        case AST_BLOCK:
             for (ASTNodeList *cur = node->as.block.statements; cur; cur = cur->next) {
                gen(cur->node);
            }
            break;

        case AST_VAR_DECL:
            // Example: var x = 10;
            // 1. Calculate the init value (e.g., 10) -> Pushes to stack
            if (node->as.var_decl.init) {
                gen(node->as.var_decl.init);
            } else {
                emit("PUSH", 0, "default init");
            }
            // 2. Store it in the variable's slot
            emit("STORE", get_var_index(node->as.var_decl.name), node->as.var_decl.name);
            break;

        case AST_ASSIGN:
            // Example: x = y + 1;
            gen(node->as.assign.value); // Generate code for (y+1), result is on stack
            emit("STORE", get_var_index(node->as.assign.name), node->as.assign.name);
            break;

        case AST_INT:
            emit("PUSH", node->as.int_lit.value, NULL);
            break;

        case AST_IDENT:
            emit("LOAD", get_var_index(node->as.ident.name), node->as.ident.name);
            break;

        case AST_BINOP:
            gen(node->as.binop.left);
            gen(node->as.binop.right);
            // Stack: [Left, Right]
            switch (node->as.binop.op) {
                case '+': emit("ADD", -1, NULL); break;
                case '-': emit("SUB", -1, NULL); break;
                case '*': emit("MUL", -1, NULL); break;
                case '/': emit("DIV", -1, NULL); break;
                case '<': emit("LT", -1, NULL); break;
                case '>': emit("GT", -1, NULL); break;
                case LE:  emit("LE", -1, NULL); break;
                case GE:  emit("GE", -1, NULL); break;
                case EQ:  emit("EQ", -1, NULL); break;
                case NEQ: emit("NEQ", -1, NULL); break;
            }
            break;

        case AST_IF: {
            int L_else = new_label();
            int L_end  = new_label();

            gen(node->as.if_stmt.cond);     // Calc condition
            emit("JZ", L_else, "if false jump"); // Jump if 0

            gen(node->as.if_stmt.then_branch);
            emit("JMP", L_end, "jump over else");

            emit_label(L_else);
            if (node->as.if_stmt.else_branch) {
                gen(node->as.if_stmt.else_branch);
            }

            emit_label(L_end);
            break;
        }

        case AST_WHILE: {
            int L_start = new_label();
            int L_end   = new_label();

            emit_label(L_start);
            gen(node->as.while_stmt.cond);
            emit("JZ", L_end, "exit loop");

            gen(node->as.while_stmt.body);
            emit("JMP", L_start, "loop back");

            emit_label(L_end);
            break;
        }

        default:
            break;
    }
}

void generate_asm(ASTNode *root, const char *filename) {
    out_file = fopen(filename, "w");
    if (!out_file) {
        perror("Failed to open output .asm file");
        return;
    }

    // Reset state for a fresh compilation
    var_count = 0;
    label_counter = 0;

    printf("[IR] Generating assembly to %s ...\n", filename);
    
    // Start Recursive Generation
    gen(root);

    emit("HALT", -1, NULL);

    fclose(out_file);
    printf("[IR] Done.\n");
}