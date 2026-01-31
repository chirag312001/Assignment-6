#ifndef LOADER_H
#define LOADER_H

#include "vm.h"

/* loader and validation helpers (raw bytecode) */
unsigned char *load_bytecode(const char *file, int *size);
int vm_validate(Program *p);

#endif
