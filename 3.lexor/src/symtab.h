#ifndef SYMTAB_H
#define SYMTAB_H

typedef struct SymTab SymTab;

/* Create a new scope, optionally linked to a parent scope. */
SymTab *symtab_create(SymTab *parent);
/* Push a new child scope on top of the current one. */
SymTab *symtab_push(SymTab *current);
/* Pop the current scope and free its entries. */
SymTab *symtab_pop(SymTab *current);

/* Declare a variable in the current scope; returns 0 on success. */
int symtab_declare(SymTab *tab, const char *name);
/* Check if a variable is declared in any active scope. */
int symtab_is_declared(SymTab *tab, const char *name);
/* Set a variable's value in the nearest scope where it exists. */
int symtab_set(SymTab *tab, const char *name, int value);
/* Get a variable's value from the nearest scope where it exists. */
int symtab_get(SymTab *tab, const char *name, int *out_value);
/* Mark a variable as unknown (non-constant value). */
int symtab_mark_unknown(SymTab *tab, const char *name);
/* Print variables from the given scope. */
void symtab_dump(const SymTab *tab);

#endif
