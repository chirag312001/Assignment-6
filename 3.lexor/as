CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g -Ibuild
LEX = flex
YACC = bison

BIN = parser
SRC_DIR = src
BUILD_DIR = build

LEX_SRC = $(SRC_DIR)/lexer.l
YACC_SRC = $(SRC_DIR)/parser.y
AST_SRC = $(SRC_DIR)/ast.c
SYMTAB_SRC = $(SRC_DIR)/symtab.c
EVAL_SRC = $(SRC_DIR)/eval.c
MAIN_SRC = $(SRC_DIR)/main.c

YACC_OUT_C = $(BUILD_DIR)/parser.tab.c
YACC_OUT_H = $(BUILD_DIR)/parser.tab.h
LEX_OUT_C = $(BUILD_DIR)/lex.yy.c

OBJS = \
	$(BUILD_DIR)/parser.tab.o \
	$(BUILD_DIR)/lex.yy.o \
	$(BUILD_DIR)/ast.o \
	$(BUILD_DIR)/symtab.o \
	$(BUILD_DIR)/eval.o \
	$(BUILD_DIR)/main.o

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(YACC_OUT_C) $(YACC_OUT_H): $(YACC_SRC) | $(BUILD_DIR)
	$(YACC) -d -o $(YACC_OUT_C) $(YACC_SRC)

$(LEX_OUT_C): $(LEX_SRC) $(YACC_OUT_H) | $(BUILD_DIR)
	$(LEX) -o $(LEX_OUT_C) $(LEX_SRC)

$(BUILD_DIR)/parser.tab.o: $(YACC_OUT_C)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c $(YACC_OUT_C) -o $@

$(BUILD_DIR)/lex.yy.o: $(LEX_OUT_C)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c $(LEX_OUT_C) -o $@

$(BUILD_DIR)/ast.o: $(AST_SRC) $(SRC_DIR)/ast.h
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $(AST_SRC) -o $@

$(BUILD_DIR)/symtab.o: $(SYMTAB_SRC) $(SRC_DIR)/symtab.h
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $(SYMTAB_SRC) -o $@

$(BUILD_DIR)/eval.o: $(EVAL_SRC) $(SRC_DIR)/eval.h $(SRC_DIR)/ast.h $(SRC_DIR)/symtab.h $(YACC_OUT_H)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c $(EVAL_SRC) -o $@

$(BUILD_DIR)/main.o: $(MAIN_SRC) $(SRC_DIR)/ast.h $(SRC_DIR)/eval.h
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(BUILD_DIR) -c $(MAIN_SRC) -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN)

.PHONY: clean
