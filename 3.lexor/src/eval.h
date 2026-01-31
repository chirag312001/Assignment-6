#ifndef EVAL_H
#define EVAL_H

#include "ast.h"
#include "symtab.h"

/* Evaluate only constant expressions and report semantic errors. Returns 0 on success. */
int eval_program(ASTNode *root, SymTab **out_globals);

#endif
