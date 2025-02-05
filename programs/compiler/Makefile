# compiles the compiler

CC = gcc
CFLAGS = -Wall -Wextra -g

BIN = compiler
SRC = compiler.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN)
