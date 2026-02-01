#ifndef EXEC_H
#define EXEC_H

#include "vm.h"


void vm_run(Program *p);
int vm_step(Program *p);


#endif
