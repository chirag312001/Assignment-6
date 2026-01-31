%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.tab.h"


static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    memcpy(p, s, n);
    return p;
}
%}

%option noyywrap
%option noinput nounput
%option yylineno

%x COMMENT

%%

[ \t\r\n]+              ;


"//".*                  ;

"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             ;
<COMMENT>.              ;
<COMMENT><<EOF>>        {
                            fprintf(stderr,
                              "Unterminated comment at line %d\n",
                              yylineno);
                            exit(1);
                        }


"var"                   { return VAR; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }


"=="                    { return EQ; }
"!="                    { return NEQ; }
"<="                    { return LE; }
">="                    { return GE; }


[0-9]+                  {
                            yylval.ival = atoi(yytext);
                            return INTEGER;
                        }

[a-zA-Z_][a-zA-Z0-9_]*  {
                            yylval.sval = xstrdup(yytext);
                            return IDENTIFIER;
                        }


"<"                     { return '<'; }
">"                     { return '>'; }
"="                     { return '='; }
"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }
";"                     { return ';'; }


.                       {
                            fprintf(stderr,
                              "Lexer error at line %d: %s\n",
                              yylineno, yytext);
                            exit(1);
                        }
%%