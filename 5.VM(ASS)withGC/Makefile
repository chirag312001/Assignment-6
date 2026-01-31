CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -g

ASM_SRC = assembler_c/assembler.c
VM_SRC  = VM/vm.c VM/stack.c VM/loader.c VM/exec.c VM/include/value.c VM/include/object.c main.c debugger/debugger.c

# to test the garbage collector
GC_TEST_SRC = VM/vm.c VM/stack.c VM/loader.c VM/exec.c VM/include/value.c VM/include/object.c VM/test.c

ASM_BIN = assembler
VM_BIN  = bvm
GC_TEST_BIN = test_gc_bin
TEST_SCRIPT = test/simple/run_vm_tests.sh
GC_SIMPLE_TEST_BIN = test_gc
GC_SIMPLE_TEST_SRC = test.c VM/include/value.c VM/include/object.c VM/vm.c VM/stack.c

all: $(ASM_BIN) $(VM_BIN)

$(ASM_BIN): $(ASM_SRC)
	$(CC) $(CFLAGS) $(ASM_SRC) -o $(ASM_BIN)

$(VM_BIN): $(VM_SRC)
	$(CC) $(CFLAGS) -I./VM  -I./VM/include  $(VM_SRC) -o $(VM_BIN)

test_gc: $(GC_TEST_SRC)
	$(CC) $(CFLAGS) -I./VM -I./VM/include $(GC_TEST_SRC) -o $(GC_TEST_BIN)
	./$(GC_TEST_BIN)

gc_test_simple: $(GC_SIMPLE_TEST_SRC)
	$(CC) $(CFLAGS) -I./VM -I./VM/include $(GC_SIMPLE_TEST_SRC) -o $(GC_SIMPLE_TEST_BIN)
	./$(GC_SIMPLE_TEST_BIN)

clean:
	rm -f $(ASM_BIN) $(VM_BIN) test1.byc

test: all
	$(TEST_SCRIPT)

.PHONY: all clean test
