CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O2

# Targets
TARGET_TEST = explicit_test
OBJS = explicit.o
TEST_OBJS = explicit_test.o $(OBJS)
HEADERS = explicit.h

# Default target
.PHONY: all
all: $(TARGET_TEST)

# Build the test executable
$(TARGET_TEST): $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS)

# Compile object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
.PHONY: clean
clean:
	rm -f $(TEST_OBJS) $(TARGET_TEST)

# Test run
.PHONY: run
run: $(TARGET_TEST)
	./$(TARGET_TEST)
