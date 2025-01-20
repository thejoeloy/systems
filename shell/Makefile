# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
LDFLAGS = -lm

# Directories
SRCDIR = src
BUILDDIR = build
BINDIR = bin
TESTDIR = test

# Target executable
TARGET = $(BINDIR)/shell

# Source and object files
TEST_SOURCES = $(wildcard $(SRCDIR)/*.c)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.c, $(BUILDDIR)/%.o, $(TEST_SOURCES))

# Default target
all: $(TARGET)

# Build target executables
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the shell
run: $(TARGET)
	@echo "Running the shell..."
	$(TARGET)

# Build and run tests
test: $(OBJECTS) $(TEST_OBJECTS)
	@echo "Running tests..."
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/test_runner $(LD_FLAGS)
	$(BINDIR)/test_runner

# Clean up build directories
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILDDIR) $(BINDIR)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all run test clean rebuild
