#ifndef DEBUGGER_H
#define DEBUGGER_H
#include "../VM/vm.h"

void debug_start(Program *p);

void add_breakpoint(int addr);

#endif