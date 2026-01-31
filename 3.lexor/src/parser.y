%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ast.h"

extern int yylex(void);
extern int yylineno;
void yyerror(const char *s);

ASTNode *root = NULL;
%}



%code requires {
#include "ast.h"
}

%start program
%define parse.error verbose


%union {
    int ival;
    char *sval;
    ASTNode *node;
    ASTNodeList *list;
}

%token VAR IF ELSE WHILE
%token <sval> IDENTIFIER
%token <ival> INTEGER
%token EQ NEQ LE GE

%type <node> program statement variable_decl assignment if_statement while_statement block
%type <node> expression equality comparison term factor unary primary
%type <list> statement_list

%nonassoc IFX
%nonassoc ELSE
%right UMINUS

%%

program
    : statement_list { $$ = ast_make_program($1, yylineno); root = $$; }
    ;

statement_list
    : /* empty */ { $$ = NULL; }
    | statement_list statement { $$ = ast_list_append($1, $2); }
    ;

statement
    : variable_decl { $$ = $1; }
    | assignment { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | block { $$ = $1; }
    |expression '=' expression ';'
        {
            yyerror("invalid assignment target");
            YYERROR;
        }
    ;

block
    : '{' statement_list '}' { $$ = ast_make_block($2, yylineno); }
    ;

variable_decl
    : VAR IDENTIFIER ';' { $$ = ast_make_var_decl($2, NULL, yylineno); }
    | VAR IDENTIFIER '=' expression ';' { $$ = ast_make_var_decl($2, $4, yylineno); }
    ;

assignment
    : IDENTIFIER '=' expression ';' { $$ = ast_make_assign($1, $3, yylineno); }
    ;

if_statement
    : IF '(' expression ')' statement %prec IFX { $$ = ast_make_if($3, $5, NULL, yylineno); }
    | IF '(' expression ')' statement ELSE statement { $$ = ast_make_if($3, $5, $7, yylineno); }
    ;

while_statement
    : WHILE '(' expression ')' statement { $$ = ast_make_while($3, $5, yylineno); }
    ;

expression
    : equality { $$ = $1; }
    ;

equality
    : comparison { $$ = $1; }
    | equality EQ comparison { $$ = ast_make_binop(EQ, $1, $3, yylineno); }
    | equality NEQ comparison { $$ = ast_make_binop(NEQ, $1, $3, yylineno); }
    ;

comparison
    : term { $$ = $1; }
    | comparison '<' term { $$ = ast_make_binop('<', $1, $3, yylineno); }
    | comparison '>' term { $$ = ast_make_binop('>', $1, $3, yylineno); }
    | comparison LE term { $$ = ast_make_binop(LE, $1, $3, yylineno); }
    | comparison GE term { $$ = ast_make_binop(GE, $1, $3, yylineno); }
    ;

term
    : factor { $$ = $1; }
    | term '+' factor { $$ = ast_make_binop('+', $1, $3, yylineno); }
    | term '-' factor { $$ = ast_make_binop('-', $1, $3, yylineno); }
    ;

factor
    : unary { $$ = $1; }
    | factor '*' unary { $$ = ast_make_binop('*', $1, $3, yylineno); }
    | factor '/' unary { $$ = ast_make_binop('/', $1, $3, yylineno); }
    ;

unary
    : '+' unary %prec UMINUS { $$ = ast_make_unop('+', $2, yylineno); }
    | '-' unary %prec UMINUS { $$ = ast_make_unop('-', $2, yylineno); }
    | primary { $$ = $1; }
    ;

primary
    : INTEGER { $$ = ast_make_int($1, yylineno); }
    | IDENTIFIER { $$ = ast_make_ident($1, yylineno); }
    | '(' expression ')' { $$ = $2; }
    ;

%%

extern char *yytext;

void yyerror(const char *s) {
    if (yytext == NULL || yytext[0] == '\0') {
        fprintf(stderr,"Syntax error at line %d: unexpected end of input\n",yylineno);
        return;
    }
    if (s && strcmp(s, "syntax error") != 0) {
        fprintf(stderr,"Syntax error at line %d: %s\n",yylineno, s);
    } else {
        fprintf(stderr,"Syntax error at line %d: unexpected token '%s'\n",yylineno, yytext);
    }
}
