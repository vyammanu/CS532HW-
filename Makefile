# Makefile for building the program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Source files
SRC = unix.c

# Object files
OBJ = $(SRC:.c=.o)

# Executable name
TARGET = program

.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile C source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ) $(TARGET)
