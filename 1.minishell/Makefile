CC = gcc
# CFLAGS for Shell (Includes Lexer build paths)
CFLAGS = -O3 -I../3.lexor/src -I../3.lexor/build -Iinclude -Wno-unused-result

# Directories
SHELL_DIR = .
LEXOR_DIR = ../3.lexor/src
LEXOR_BUILD = ../3.lexor/build

# Path to the VM Project
# NOTE: The parentheses here cause issues if not quoted later!
VM_DIR = ../5.VM(ASS)withGC

# --- SHELL SOURCES ---
SHELL_SRCS = mini-shell.c include/tokenizer.c include/execute.c include/parser.c include/history.c process/process_mgmt.c

# --- LEXER SOURCES ---
LEXOR_SRCS = $(LEXOR_DIR)/main.c $(LEXOR_DIR)/ast.c $(LEXOR_DIR)/eval.c $(LEXOR_DIR)/symtab.c $(LEXOR_DIR)/ir.c

# --- OUTPUT FILES ---
TARGET_SHELL = mini-shell

# --- BISON/FLEX ---
BISON_SRC = $(LEXOR_DIR)/parser.y
FLEX_SRC = $(LEXOR_DIR)/lexer.l
BISON_C = $(LEXOR_BUILD)/parser.tab.c
BISON_H = $(LEXOR_BUILD)/parser.tab.h
FLEX_C = $(LEXOR_BUILD)/lex.yy.c

# --- BUILD RULES ---

.PHONY: all clean vm_build

# Main Target: Build Shell AND call the VM Makefile
all: $(TARGET_SHELL) vm_build

# 1. Recursive Make: Go to VM_DIR and run 'make'
# FIX: Added quotes around "$(VM_DIR)" to handle parentheses in folder name
vm_build:
	@echo "--- Building VM and Assembler ---"
	$(MAKE) -C "$(VM_DIR)"

# 2. Create Build Dir for Lexer
$(LEXOR_BUILD):
	mkdir -p $(LEXOR_BUILD)

# 3. Run Bison
$(BISON_C) $(BISON_H): $(BISON_SRC) | $(LEXOR_BUILD)
	bison -d -o $(BISON_C) $(BISON_SRC)

# 4. Run Flex
$(FLEX_C): $(FLEX_SRC) $(BISON_H) | $(LEXOR_BUILD)
	flex -o $(FLEX_C) $(FLEX_SRC)

# 5. Compile Mini-Shell
$(TARGET_SHELL): $(BISON_C) $(FLEX_C) $(SHELL_SRCS) $(LEXOR_SRCS)
	@echo "--- Building Mini-Shell ---"
	$(CC) $(CFLAGS) -o $(TARGET_SHELL) \
		$(SHELL_SRCS) \
		$(LEXOR_SRCS) \
		$(BISON_C) \
		$(FLEX_C)

# --- CLEAN ---
clean:
	rm -f $(TARGET_SHELL)
	rm -rf $(LEXOR_BUILD)
	@echo "--- Cleaning VM Project ---"
	# FIX: Added quotes here too
	$(MAKE) -C "$(VM_DIR)" clean