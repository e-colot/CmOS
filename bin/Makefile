# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g -I../headers
LDFLAGS =

# Directories
SRC_DIR = ../src
OBJ_DIR = .
HEADER_DIR = ../headers

# Find source files and corresponding object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


# Output executable
TARGET = CmOS

# Default rule
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile each .c file into an .o file (ensures dependencies on headers)
%.o: $(SRC_DIR)/%.c $(HEADER_DIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
