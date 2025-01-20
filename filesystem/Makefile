# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
LDFLAGS = 

# Directories
SRCDIR = src
BUILDDIR = build
BINDIR = bin
TESTDIR = $(SRCDIR)/test

# Target executable
TARGET = $(BINDIR)/filesystem

# Source and object files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

# Test sources and objects
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(patsubst $(TESTDIR)/%.c, $(BUILDDIR)/%.o, $(TEST_SOURCES))

# Default target
all: $(TARGET)

# Build target executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test object files
$(BUILDDIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests
test: $(OBJECTS) $(TEST_OBJECTS)
	@echo "Running tests..."
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/test_runner $(LDFLAGS)
	$(BINDIR)/test_runner

# Clean up
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILDDIR) $(BINDIR)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all test clean rebuild
