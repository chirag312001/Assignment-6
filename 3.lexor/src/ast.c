#include "ast.h"
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *op_to_string(int op){
    switch (op) {
        case EQ:  return "==";
        case NEQ: return "!=";
        case LE:  return "<=";
        case GE:  return ">=";
        case '<': return "<";
        case '>': return ">";
        case '+': return "+";
        case '-': return "-";
        case '*': return "*";
        case '/': return "/";
        default:  return "?";
    }
}
/* Allocate a zeroed node and fill common fields. */
static ASTNode *ast_alloc(ASTNodeType type, int line) {
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node) {
        perror("calloc");
        exit(1);
    }
    node->type = type;
    node->line = line;
    return node;
}

/* Append to the end to preserve statement order. */
ASTNodeList *ast_list_append(ASTNodeList *list, ASTNode *node) {
    ASTNodeList *item = (ASTNodeList *)calloc(1, sizeof(ASTNodeList));
    if (!item) {
        perror("calloc");
        exit(1);
    }
    item->node = node;

    if (!list) {
        return item;
    }

    ASTNodeList *cur = list;
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = item;
    return list;
}

/* Free a list and its nodes. */
void ast_list_free(ASTNodeList *list) {
    while (list) {
        ASTNodeList *next = list->next;
        ast_free(list->node);
        free(list);
        list = next;
    }
}

ASTNode *ast_make_program(ASTNodeList *list, int line) {
    ASTNode *node = ast_alloc(AST_PROGRAM, line);
    node->as.program.statements = list;
    return node;
}

ASTNode *ast_make_block(ASTNodeList *list, int line) {
    ASTNode *node = ast_alloc(AST_BLOCK, line);
    node->as.block.statements = list;
    return node;
}

ASTNode *ast_make_var_decl(char *name, ASTNode *init, int line) {
    ASTNode *node = ast_alloc(AST_VAR_DECL, line);
    node->as.var_decl.name = name;
    node->as.var_decl.init = init;
    return node;
}

ASTNode *ast_make_assign(char *name, ASTNode *value, int line) {
    ASTNode *node = ast_alloc(AST_ASSIGN, line);
    node->as.assign.name = name;
    node->as.assign.value = value;
    return node;
}

ASTNode *ast_make_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line) {
    ASTNode *node = ast_alloc(AST_IF, line);
    node->as.if_stmt.cond = cond;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode *ast_make_while(ASTNode *cond, ASTNode *body, int line) {
    ASTNode *node = ast_alloc(AST_WHILE, line);
    node->as.while_stmt.cond = cond;
    node->as.while_stmt.body = body;
    return node;
}

ASTNode *ast_make_binop(int op, ASTNode *left, ASTNode *right, int line) {
    ASTNode *node = ast_alloc(AST_BINOP, line);
    node->as.binop.op = op;
    node->as.binop.left = left;
    node->as.binop.right = right;
    return node;
}

ASTNode *ast_make_unop(int op, ASTNode *expr, int line) {
    ASTNode *node = ast_alloc(AST_UNOP, line);
    node->as.unop.op = op;
    node->as.unop.expr = expr;
    return node;
}

ASTNode *ast_make_int(int value, int line) {
    ASTNode *node = ast_alloc(AST_INT, line);
    node->as.int_lit.value = value;
    return node;
}

ASTNode *ast_make_ident(char *name, int line) {
    ASTNode *node = ast_alloc(AST_IDENT, line);
    node->as.ident.name = name;
    return node;
}

/* Recursively free the AST. */
void ast_free(ASTNode *node) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case AST_PROGRAM:
            ast_list_free(node->as.program.statements);
            break;
        case AST_BLOCK:
            ast_list_free(node->as.block.statements);
            break;
        case AST_VAR_DECL:
            free(node->as.var_decl.name);
            ast_free(node->as.var_decl.init);
            break;
        case AST_ASSIGN:
            free(node->as.assign.name);
            ast_free(node->as.assign.value);
            break;
        case AST_IF:
            ast_free(node->as.if_stmt.cond);
            ast_free(node->as.if_stmt.then_branch);
            ast_free(node->as.if_stmt.else_branch);
            break;
        case AST_WHILE:
            ast_free(node->as.while_stmt.cond);
            ast_free(node->as.while_stmt.body);
            break;
        case AST_BINOP:
            ast_free(node->as.binop.left);
            ast_free(node->as.binop.right);
            break;
        case AST_UNOP:
            ast_free(node->as.unop.expr);
            break;
        case AST_INT:
            break;
        case AST_IDENT:
            free(node->as.ident.name);
            break;
        default:
            break;
    }

    free(node);
}


void indent(int level){
    for (int i = 0; i < level; i++)
        printf("  ");
}


void pretty_print_list(ASTNodeList *list, int level){
    
    while (list) {
        pretty_print_node(list->node, level);
        list = list->next;
    }
}


void pretty_print_node(ASTNode *node, int level){
    if (!node)
        return;

    switch (node->type) {

    /* ---------------- PROGRAM ---------------- */
    case AST_PROGRAM:
        pretty_print_list(node->as.program.statements, level);
        break;

    /* ---------------- BLOCK ---------------- */
    case AST_BLOCK:
        indent(level);
        printf("{\n");
        pretty_print_list(node->as.block.statements, level + 1);
        indent(level);
        printf("}\n");
        break;

    /* ---------------- VAR DECL ---------------- */
    case AST_VAR_DECL:
        indent(level);
        printf("VarDecl %s", node->as.var_decl.name);
        if (node->as.var_decl.init) {
            printf(" =\n");
            pretty_print_node(node->as.var_decl.init, level + 1);
        } else {
            printf("\n");
        }
        break;

    /* ---------------- ASSIGN ---------------- */
    case AST_ASSIGN:
        indent(level);
        printf("Assign %s =\n", node->as.assign.name);
        pretty_print_node(node->as.assign.value, level + 1);
        break;

    /* ---------------- IF ---------------- */
    case AST_IF:
        indent(level);
        printf("If\n");

        indent(level + 1);
        printf("Condition:\n");
        pretty_print_node(node->as.if_stmt.cond, level + 2);

        indent(level + 1);
        printf("Then:\n");
        pretty_print_node(node->as.if_stmt.then_branch, level + 2);

        if (node->as.if_stmt.else_branch) {
            indent(level + 1);
            printf("Else:\n");
            pretty_print_node(node->as.if_stmt.else_branch, level + 2);
        }
        break;

    /* ---------------- WHILE ---------------- */
    case AST_WHILE:
        indent(level);
        printf("While\n");

        indent(level + 1);
        printf("Condition:\n");
        pretty_print_node(node->as.while_stmt.cond, level + 2);

        indent(level + 1);
        printf("Body:\n");
        pretty_print_node(node->as.while_stmt.body, level + 2);
        break;

    /* ---------------- BINARY OP ---------------- */
    case AST_BINOP:
        indent(level);
        printf("BinOp '%s'\n", op_to_string(node->as.binop.op));
        pretty_print_node(node->as.binop.left, level + 1);
        pretty_print_node(node->as.binop.right, level + 1);
        break;

    /* ---------------- UNARY OP ---------------- */
    case AST_UNOP:
        indent(level);
        printf("UnOp '%s'\n", op_to_string(node->as.unop.op));
        pretty_print_node(node->as.unop.expr, level + 1);
        break;

    /* ---------------- INT ---------------- */
    case AST_INT:
        indent(level);
        printf("Int %d\n", node->as.int_lit.value);
        break;

    /* ---------------- IDENT ---------------- */
    case AST_IDENT:
        indent(level);
        printf("Var %s\n", node->as.ident.name);
        break;

    /* ---------------- DEFAULT ---------------- */
    default:
        indent(level);
        printf("<<Unknown AST Node>>\n");
        break;
    }
}

void ast_pretty_print(ASTNode *root){
    pretty_print_node(root, 0);
}

/* Return 1 and set out if the binary op can be folded safely. */
static int fold_binop(int op, int left, int right, int *out) {
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
            return 0;
    }
}

/* Return 1 and set out if the unary op can be folded safely. */
static int fold_unop(int op, int value, int *out) {
    if (op == '+') {
        *out = value;
        return 1;
    }
    if (op == '-') {
        *out = -value;
        return 1;
    }
    return 0;
}

static void fold_list(ASTNodeList *list) {
    while (list) {
        ast_fold_constants(list->node);
        list = list->next;
    }
}

void ast_fold_constants(ASTNode *node) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case AST_PROGRAM:
            fold_list(node->as.program.statements);
            break;
        case AST_BLOCK:
            fold_list(node->as.block.statements);
            break;
        case AST_VAR_DECL:
            if (node->as.var_decl.init) {
                ast_fold_constants(node->as.var_decl.init);
            }
            break;
        case AST_ASSIGN:
            ast_fold_constants(node->as.assign.value);
            break;
        case AST_IF:
            ast_fold_constants(node->as.if_stmt.cond);
            ast_fold_constants(node->as.if_stmt.then_branch);
            if (node->as.if_stmt.else_branch) {
                ast_fold_constants(node->as.if_stmt.else_branch);
            }
            break;
        case AST_WHILE:
            ast_fold_constants(node->as.while_stmt.cond);
            ast_fold_constants(node->as.while_stmt.body);
            break;
        case AST_UNOP: {
            ast_fold_constants(node->as.unop.expr);
            if (node->as.unop.expr && node->as.unop.expr->type == AST_INT) {
                int folded = 0;
                if (fold_unop(node->as.unop.op, node->as.unop.expr->as.int_lit.value, &folded)) {
                    ast_free(node->as.unop.expr);
                    node->type = AST_INT;
                    node->as.int_lit.value = folded;
                }
            }
            break;
        }
        case AST_BINOP: {
            ast_fold_constants(node->as.binop.left);
            ast_fold_constants(node->as.binop.right);
            if (node->as.binop.left && node->as.binop.right &&
                node->as.binop.left->type == AST_INT &&
                node->as.binop.right->type == AST_INT) {
                int folded = 0;
                if (fold_binop(node->as.binop.op,
                               node->as.binop.left->as.int_lit.value,
                               node->as.binop.right->as.int_lit.value,
                               &folded)) {
                    ast_free(node->as.binop.left);
                    ast_free(node->as.binop.right);
                    node->type = AST_INT;
                    node->as.int_lit.value = folded;
                }
            }
            break;
        }
        default:
            break;
    }
}

