# Variables
CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ_FILES = builtins.o executor.o jobs.o parser.o signals.o shell.o

# Default target
all: shell

# Rule to build the final executable
shell: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $(OBJ_FILES)

# Rule to compile object files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Special case for shell.o, since it doesn't have a header file
shell.o: shell.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f *.o shell

.PHONY: all clean
