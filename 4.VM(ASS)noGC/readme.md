
# Bytecode Virtual Machine (BVM)


## This project contains:
1. An Assembler that converts .asm files into .byc bytecode
2. A Stack-based Virtual Machine (VM) that executes .byc files

---


## Build Instructions


- Build the Assembler and vm

cd assembler
make

- This generates:
./assembler ./bvm


- Running the Assembler

- The assembler converts assembly (.asm) files to bytecode (.byc)

- Syntax:
./assembler input.asm output.byc

- Example:

./assembler test/test_arithmetic.asm arithmetic.byc

- Output:

arithmetic.byc


- Running the Virtual Machine


- The VM executes bytecode (.byc) files

- Syntax:
 ./bvm <program.byc>

- Example:

./bvm arithmetic.byc

--- 

## Executing Provided Test Programs


1. Arithmetic Test

./assembler test/test_arithmetic.asm arithmetic.byc

./bvm arithmetic.byc

2. Loop Test

./assembler test/test_loop.asm loop.byc

./bvm loop.byc

# 3. Function Call Test

./assembler test/test_function.asm function.byc

./bvm function.byc

---

#### Notes


- The VM accepts only .byc files
- Invalid bytecode is rejected before execution
- Runtime errors (stack underflow, division by zero,
- invalid jumps, unknown opcodes) are handled by the VM
- Execution stops cleanly when HALT is encountered

---

#### Execution Flow


   Assembly (.asm)
          |
          v
     Assembler
          |
          v
     Bytecode (.byc)
          |
          v
   Virtual Machine
          |
          v
       Execution


