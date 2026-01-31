# Bytecode Virtual Machine (BVM) + Mark-Sweep GC

## Overview
This project contains:
1. An assembler that converts `.asm` files into `.byc` bytecode.
2. A stack-based virtual machine that executes `.byc` files.
3. A stop-the-world mark-sweep garbage collector integrated into the VM.

---

## Build Instructions

```bash
make
```

This generates:
- `./assembler`
- `./bvm`

---

## Running the Assembler

```bash
./assembler input.asm output.byc
```

Example:
```bash
./assembler test/arithmetic.asm arithmetic.byc
```

---

## Running the Virtual Machine

```bash
./bvm <program.byc>
```

Example:
```bash
./bvm arithmetic.byc
```

---

## Garbage Collection

GC is explicit. The VM exposes roots via the operand stack and memory.
The current setup triggers GC from `main.c` after VM execution.

To run GC tests:
```bash
make test_gc
make gc_test_simple
```

---

## Tests

### VM opcode test suite
```bash
bash test/simple/run_vm_tests.sh
```

### GC interactive test suite
```bash
make test_gc
```

### GC automated test suite
```bash
make gc_test_simple
```

---

## Notes
- The VM accepts only `.byc` files.
- Invalid bytecode is rejected before execution.
- Runtime errors (stack underflow, division by zero, invalid jumps, unknown opcodes)
  are handled by the VM.
- Execution stops cleanly when `HALT` is encountered.

---

## Execution Flow

Assembly (`.asm`)
   |
   v
Assembler
   |
   v
Bytecode (`.byc`)
   |
   v
Virtual Machine
   |
   v
Execution
