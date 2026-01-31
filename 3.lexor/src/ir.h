// 3.lexor/src/ir.h
#ifndef IR_H
#define IR_H

#include "ast.h"

// Generates the .asm file from the AST
// filename: The output file name (e.g., "output.asm")
void generate_asm(ASTNode *root, const char *filename);

#endif