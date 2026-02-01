CC = gcc
# Added VM include paths so the Shell can find debugger.h and vm.h
# Quotes around the include paths are necessary for the compiler to find the directories
CFLAGS = -O3 -I../3.lexor/src -I../3.lexor/build -Iinclude -Wno-unused-result \
         -I"$(VM_DIR)" -I"$(VM_DIR)/VM" -I"$(VM_DIR)/VM/include"

# Directories
SHELL_DIR = .
LEXOR_DIR = ../3.lexor/src
LEXOR_BUILD = ../3.lexor/build

# Use quotes here to define the path, but the real fix happens in the command line expansion
VM_DIR = ../5.VM(ASS)withGC

# --- VM SOURCES ---
# We list them individually with quotes to ensure the Shell doesn't trip on the ()
VM_CORE = "$(VM_DIR)/VM/vm.c" \
          "$(VM_DIR)/VM/stack.c" \
          "$(VM_DIR)/VM/loader.c" \
          "$(VM_DIR)/VM/exec.c" \
          "$(VM_DIR)/VM/include/value.c" \
          "$(VM_DIR)/VM/include/object.c" \
          "$(VM_DIR)/debugger/debugger.c"

# --- SHELL SOURCES ---
SHELL_SRCS = mini-shell.c include/tokenizer.c include/execute.c include/parser.c include/history.c process/process_mgmt.c

# --- LEXER SOURCES ---
LEXOR_SRCS = $(LEXOR_DIR)/main.c $(LEXOR_DIR)/ast.c $(LEXOR_DIR)/eval.c $(LEXOR_DIR)/symtab.c $(LEXOR_DIR)/ir.c

TARGET_SHELL = mini-shell
BISON_C = $(LEXOR_BUILD)/parser.tab.c
FLEX_C = $(LEXOR_BUILD)/lex.yy.c

.PHONY: all clean vm_build

# 1. First call VM Makefile, then build Shell
all: vm_build $(TARGET_SHELL)

vm_build:
	@echo "--- Building VM and Assembler ---"
	$(MAKE) -C "$(VM_DIR)"

$(LEXOR_BUILD):
	mkdir -p $(LEXOR_BUILD)

$(BISON_C): $(LEXOR_DIR)/parser.y | $(LEXOR_BUILD)
	bison -d -o $(BISON_C) $(LEXOR_DIR)/parser.y

$(FLEX_C): $(LEXOR_DIR)/lexer.l | $(LEXOR_BUILD)
	flex -o $(FLEX_C) $(LEXOR_DIR)/lexer.l

# 5. Compile Mini-Shell with VM Sources linked in
# Note the quotes around the VM core files to handle the syntax error
$(TARGET_SHELL): $(BISON_C) $(FLEX_C) $(SHELL_SRCS) $(LEXOR_SRCS)
	@echo "--- Building Integrated Mini-Shell ---"
	$(CC) $(CFLAGS) -o $(TARGET_SHELL) \
		$(SHELL_SRCS) \
		$(LEXOR_SRCS) \
		$(VM_CORE) \
		$(BISON_C) \
		$(FLEX_C)

clean:
	rm -f $(TARGET_SHELL)
	rm -rf $(LEXOR_BUILD)
	$(MAKE) -C "$(VM_DIR)" clean