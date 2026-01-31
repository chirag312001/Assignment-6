CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I.

SRC = \
	main.c \
	common/common.c \
	shell/shell.c \
	shell/proc_table.c \
	parser/parser.c \
	ir/ir.c \
	vm/vm.c \
	debug/debug.c \
	gc/gc.c

OBJ = $(SRC:.c=.o)

TARGET = system

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
