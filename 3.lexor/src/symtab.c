#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Single-linked list of variables for one scope. */
typedef struct SymTabEntry {
    char *name;
    int value;
    int has_value;
    struct SymTabEntry *next;
} SymTabEntry;

/* Each symbol table node represents one scope with a parent link. */
struct SymTab {
    SymTabEntry *head;
    struct SymTab *parent;
};

/* Local strdup replacement to keep builds C11-friendly. */
static char *xstrdup(const char *src) {
    size_t len = strlen(src) + 1;
    char *copy = (char *)malloc(len);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, src, len);
    return copy;
}

/* Find a symbol in the current scope only. */
static SymTabEntry *symtab_find_entry(SymTab *tab, const char *name) {
    for (SymTabEntry *entry = tab->head; entry; entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
    }
    return NULL;
}

/* Walk up the scope chain to find a symbol. */
static SymTabEntry *symtab_find_entry_recursive(SymTab *tab, const char *name) {
    for (SymTab *cur = tab; cur; cur = cur->parent) {
        SymTabEntry *entry = symtab_find_entry(cur, name);
        if (entry) {
            return entry;
        }
    }
    return NULL;
}

SymTab *symtab_create(SymTab *parent) {
    SymTab *tab = (SymTab *)calloc(1, sizeof(SymTab));
    if (!tab) {
        return NULL;
    }
    tab->parent = parent;
    return tab;
}

SymTab *symtab_push(SymTab *current) {
    return symtab_create(current);
}

SymTab *symtab_pop(SymTab *current) {
    if (!current) {
        return NULL;
    }
    SymTab *parent = current->parent;

    /* Free entries in the current scope. */
    SymTabEntry *entry = current->head;
    while (entry) {
        SymTabEntry *next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }

    free(current);
    return parent;
}

int symtab_declare(SymTab *tab, const char *name) {
    if (!tab || !name) {
        return -1;
    }
    if (symtab_find_entry(tab, name)) {
        return -1;
    }

    /* Insert at head for O(1) push. */
    SymTabEntry *entry = (SymTabEntry *)calloc(1, sizeof(SymTabEntry));
    if (!entry) {
        return -1;
    }
    entry->name = xstrdup(name);
    if (!entry->name) {
        free(entry);
        return -1;
    }
    entry->value = 0;
    entry->has_value = 0;
    entry->next = tab->head;
    tab->head = entry;
    return 0;
}

int symtab_is_declared(SymTab *tab, const char *name) {
    return symtab_find_entry_recursive(tab, name) ? 1 : 0;
}

int symtab_set(SymTab *tab, const char *name, int value) {
    SymTabEntry *entry = symtab_find_entry_recursive(tab, name);
    if (!entry) {
        return -1;
    }
    entry->value = value;
    entry->has_value = 1;
    return 0;
}

int symtab_get(SymTab *tab, const char *name, int *out_value) {
    SymTabEntry *entry = symtab_find_entry_recursive(tab, name);
    if (!entry) {
        return -1;
    }
    if (!entry->has_value) {
        return -1;
    }
    if (out_value) {
        *out_value = entry->value;
    }
    return 0;
}

int symtab_mark_unknown(SymTab *tab, const char *name) {
    SymTabEntry *entry = symtab_find_entry_recursive(tab, name);
    if (!entry) {
        return -1;
    }
    entry->has_value = 0;
    entry->value = 0;
    return 0;
}

static void symtab_dump_entries(const SymTabEntry *entry) {
    if (!entry) {
        return;
    }
    symtab_dump_entries(entry->next);
    if (entry->has_value) {
        printf("  %s = %d\n", entry->name, entry->value);
    } else {
        printf("  %s = <unknown>\n", entry->name);
    }
}

void symtab_dump(const SymTab *tab) {
    if (!tab) {
        return;
    }
    printf("Variables:\n");
    symtab_dump_entries(tab->head);
}
