CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -g

ASM_SRC = assembler_c/assembler.c
VM_SRC  = VM/vm.c VM/stack.c VM/loader.c VM/exec.c main.c

ASM_BIN = assembler
VM_BIN  = bvm
TEST_SCRIPT = test/run_vm_tests.sh

all: $(ASM_BIN) $(VM_BIN)

$(ASM_BIN): $(ASM_SRC)
	$(CC) $(CFLAGS) $(ASM_SRC) -o $(ASM_BIN)

$(VM_BIN): $(VM_SRC)
	$(CC) $(CFLAGS) -I./VM $(VM_SRC) -o $(VM_BIN)

clean:
	rm -f $(ASM_BIN) $(VM_BIN) test1.byc

test: all
	$(TEST_SCRIPT)

.PHONY: all clean test
