CC = gcc
# ADDED -I../3.lexor/build so it can find parser.tab.h
CFLAGS = -O3 -I../3.lexor/src -I../3.lexor/build -Iinclude -Wno-unused-result

# Directories
SHELL_DIR = .
LEXOR_DIR = ../3.lexor/src
LEXOR_BUILD = ../3.lexor/build

# Source Files
SHELL_SRCS = mini-shell.c include/tokenizer.c include/execute.c include/parser.c include/history.c

# Lexer Source Files -- ADDED ir.c HERE
LEXOR_SRCS = $(LEXOR_DIR)/main.c $(LEXOR_DIR)/ast.c $(LEXOR_DIR)/eval.c $(LEXOR_DIR)/symtab.c $(LEXOR_DIR)/ir.c

# Generated Files (Bison/Flex)
BISON_SRC = $(LEXOR_DIR)/parser.y
FLEX_SRC = $(LEXOR_DIR)/lexer.l
BISON_C = $(LEXOR_BUILD)/parser.tab.c
BISON_H = $(LEXOR_BUILD)/parser.tab.h
FLEX_C = $(LEXOR_BUILD)/lex.yy.c

# Final Output
TARGET = mini-shell

all: $(TARGET)

# 1. Create Build Dir for Lexer
$(LEXOR_BUILD):
	mkdir -p $(LEXOR_BUILD)

# 2. Run Bison (Generates .c and .h)
$(BISON_C) $(BISON_H): $(BISON_SRC) | $(LEXOR_BUILD)
	bison -d -o $(BISON_C) $(BISON_SRC)

# 3. Run Flex (Depends on Bison Header)
$(FLEX_C): $(FLEX_SRC) $(BISON_H) | $(LEXOR_BUILD)
	flex -o $(FLEX_C) $(FLEX_SRC)

# 4. Compile EVERYTHING
# We compile the Shell files, Lexer files, and the generated Bison/Flex files together.
# The CFLAGS includes the build dir so 'eval.c' can find 'parser.tab.h'.
$(TARGET): $(BISON_C) $(FLEX_C) $(SHELL_SRCS) $(LEXOR_SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) \
		$(SHELL_SRCS) \
		$(LEXOR_SRCS) \
		$(BISON_C) \
		$(FLEX_C)

clean:
	rm -f $(TARGET)
	rm -rf $(LEXOR_BUILD)