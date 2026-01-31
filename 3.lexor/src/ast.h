#ifndef AST_H
#define AST_H

/* Forward declarations for tree and list nodes. */
typedef struct ASTNode ASTNode;
typedef struct ASTNodeList ASTNodeList;

/* Node kinds used by the parser and evaluator. */
typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_BINOP,
    AST_UNOP,
    AST_INT,
    AST_IDENT
} ASTNodeType;

/* Simple list container used for statement sequences. */
struct ASTNodeList {
    ASTNode *node;
    ASTNodeList *next;
};

/* Every node carries a line number for error reporting. */
struct ASTNode {
    ASTNodeType type;
    int line;
    union {
        struct {
            ASTNodeList *statements;
        } program;
        struct {
            ASTNodeList *statements;
        } block;
        struct {
            char *name;
            ASTNode *init;
        } var_decl;
        struct {
            char *name;
            ASTNode *value;
        } assign;
        struct {
            ASTNode *cond;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_stmt;
        struct {
            ASTNode *cond;
            ASTNode *body;
        } while_stmt;
        struct {
            int op;
            ASTNode *left;
            ASTNode *right;
        } binop;
        struct {
            int op;
            ASTNode *expr;
        } unop;
        struct {
            int value;
        } int_lit;
        struct {
            char *name;
        } ident;
    } as;
};

ASTNodeList *ast_list_append(ASTNodeList *list, ASTNode *node);
void ast_list_free(ASTNodeList *list);

ASTNode *ast_make_program(ASTNodeList *list, int line);
ASTNode *ast_make_block(ASTNodeList *list, int line);
ASTNode *ast_make_var_decl(char *name, ASTNode *init, int line);
ASTNode *ast_make_assign(char *name, ASTNode *value, int line);
ASTNode *ast_make_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line);
ASTNode *ast_make_while(ASTNode *cond, ASTNode *body, int line);
ASTNode *ast_make_binop(int op, ASTNode *left, ASTNode *right, int line);
ASTNode *ast_make_unop(int op, ASTNode *expr, int line);
ASTNode *ast_make_int(int value, int line);
ASTNode *ast_make_ident(char *name, int line);

void ast_free(ASTNode *node);
/* Fold constant-only expressions in place to simplify AST output. */
void ast_fold_constants(ASTNode *root);


void pretty_print_node(ASTNode *node, int level);
void pretty_print_list(ASTNodeList *list, int level);
void indent(int level);
void ast_pretty_print(ASTNode *root);

#endif
