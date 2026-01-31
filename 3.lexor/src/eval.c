#include "eval.h"
#include "symtab.h"

#include "parser.tab.h"  /* token values for ==, !=, <=, >= */

#include <stdio.h>

/* Simple semantic checker that only evaluates constant expressions. */
typedef struct {
    SymTab *scope;
    int error_count;
} EvalContext;

/* Centralized error formatting to keep output consistent. */
static void eval_error(EvalContext *ctx, int line, const char *msg) {
    fprintf(stderr, "Semantic error at line %d: %s\n", line, msg);
    ctx->error_count++;
}

static void eval_error_name(EvalContext *ctx, int line, const char *msg, const char *name) {
    fprintf(stderr, "Semantic error at line %d: %s '%s'\n", line, msg, name);
    ctx->error_count++;
}

/* Expression evaluation returns 1 only for constant expressions. */
static int eval_expr(ASTNode *node, EvalContext *ctx, int *out);

/* Statement evaluation only reports errors; it does not return a value. */
static void eval_statement(ASTNode *node, EvalContext *ctx);

static void eval_statement_list(ASTNodeList *list, EvalContext *ctx) {
    for (ASTNodeList *cur = list; cur; cur = cur->next) {
        eval_statement(cur->node, ctx);
    }
}

/* Binary operators: comparisons evaluate to 0 or 1 when both sides are constant. */
static int eval_binop(int op, int left, int right, EvalContext *ctx, int line, int *out) {
    switch (op) {
        case '+':
            *out = left + right;
            return 1;
        case '-':
            *out = left - right;
            return 1;
        case '*':
            *out = left * right;
            return 1;
        case '/':
            if (right == 0) {
                eval_error(ctx, line, "division by zero");
                *out = 0;
                return 0;
            }
            *out = left / right;
            return 1;
        case '<':
            *out = left < right;
            return 1;
        case '>':
            *out = left > right;
            return 1;
        case LE:
            *out = left <= right;
            return 1;
        case GE:
            *out = left >= right;
            return 1;
        case EQ:
            *out = left == right;
            return 1;
        case NEQ:
            *out = left != right;
            return 1;
        default:
            eval_error(ctx, line, "unknown binary operator");
            *out = 0;
            return 0;
    }
}

static int eval_expr(ASTNode *node, EvalContext *ctx, int *out) {
    if (!node) {
        if (out) {
            *out = 0;
        }
        return 0;
    }

    switch (node->type) {
        case AST_INT:
            if (out) {
                *out = node->as.int_lit.value;
            }
            return 1;
        case AST_IDENT: {
            if (!symtab_is_declared(ctx->scope, node->as.ident.name)) {
                eval_error_name(ctx, node->line, "use of undeclared variable", node->as.ident.name);
                if (out) {
                    *out = 0;
                }
                return 0;
            }
            /* Identifiers make the expression non-constant. */
            if (out) {
                *out = 0;
            }
            return 0;
        }
        case AST_UNOP: {
            int value = 0;
            if (!eval_expr(node->as.unop.expr, ctx, &value)) {
                if (out) {
                    *out = 0;
                }
                return 0;
            }
            if (node->as.unop.op == '-') {
                value = -value;
            } else if (node->as.unop.op != '+') {
                eval_error(ctx, node->line, "unknown unary operator");
                if (out) {
                    *out = 0;
                }
                return 0;
            }
            if (out) {
                *out = value;
            }
            return 1;
        }
        case AST_BINOP: {
            int left = 0;
            int right = 0;
            int left_const = eval_expr(node->as.binop.left, ctx, &left);
            int right_const = eval_expr(node->as.binop.right, ctx, &right);
            if (left_const && right_const) {
                return eval_binop(node->as.binop.op, left, right, ctx, node->line, out);
            }
            if (out) {
                *out = 0;
            }
            return 0;
        }
        default:
            eval_error(ctx, node->line, "invalid expression node");
            if (out) {
                *out = 0;
            }
            return 0;
    }
}

static void eval_statement(ASTNode *node, EvalContext *ctx) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case AST_PROGRAM:
            eval_statement_list(node->as.program.statements, ctx);
            break;
        case AST_BLOCK: {
            SymTab *prev = ctx->scope;
            SymTab *child = symtab_push(ctx->scope);
            if (!child) {
                eval_error(ctx, node->line, "failed to create scope");
                /* Fall back to current scope to keep running. */
                eval_statement_list(node->as.block.statements, ctx);
                break;
            }
            ctx->scope = child;
            eval_statement_list(node->as.block.statements, ctx);
            ctx->scope = symtab_pop(ctx->scope);
            if (!ctx->scope) {
                ctx->scope = prev;
            }
            break;
        }
        case AST_VAR_DECL: {
            if (symtab_declare(ctx->scope, node->as.var_decl.name) != 0) {
                eval_error_name(ctx, node->line, "redefinition of variable", node->as.var_decl.name);
                break;
            }
            int value = 0;
            if (node->as.var_decl.init) {
                if (eval_expr(node->as.var_decl.init, ctx, &value)) {
                    symtab_set(ctx->scope, node->as.var_decl.name, value);
                } else {
                    symtab_mark_unknown(ctx->scope, node->as.var_decl.name);
                }
            }
            break;
        }
        case AST_ASSIGN: {
            if (!symtab_is_declared(ctx->scope, node->as.assign.name)) {
                eval_error_name(ctx, node->line, "assignment to undeclared variable", node->as.assign.name);
                break;
            }
            int value = 0;
            if (eval_expr(node->as.assign.value, ctx, &value)) {
                symtab_set(ctx->scope, node->as.assign.name, value);
            } else {
                symtab_mark_unknown(ctx->scope, node->as.assign.name);
            }
            break;
        }
        case AST_IF: {
            int cond = 0;
            if (eval_expr(node->as.if_stmt.cond, ctx, &cond)) {
                if (cond) {
                    eval_statement(node->as.if_stmt.then_branch, ctx);
                } else if (node->as.if_stmt.else_branch) {
                    eval_statement(node->as.if_stmt.else_branch, ctx);
                }
            }
            /* Non-constant conditions are skipped. */
            break;
        }
        case AST_WHILE: {
            int guard_ok = 1;
            int cond = 0;
            while (guard_ok && eval_expr(node->as.while_stmt.cond, ctx, &cond) && cond) {
                eval_statement(node->as.while_stmt.body, ctx);
                /* Stop looping if errors were reported inside the body. */
                if (ctx->error_count > 0) {
                    guard_ok = 0;
                }
            }
            /* Non-constant loop conditions are skipped. */
            break;
        }
        default:
            eval_error(ctx, node->line, "invalid statement node");
            break;
    }
}

int eval_program(ASTNode *root, SymTab **out_globals) {
    EvalContext ctx;
    ctx.scope = symtab_create(NULL);
    ctx.error_count = 0;

    if (!ctx.scope) {
        fprintf(stderr, "Semantic error: failed to initialize symbol table\n");
        return 1;
    }

    /* Program-level scope lives for the whole evaluation. */
    eval_statement(root, &ctx);

    if (out_globals) {
        *out_globals = ctx.scope;
    }

    return ctx.error_count == 0 ? 0 : 1;
}
