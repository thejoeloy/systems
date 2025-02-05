CC = gcc
CFLAGS = -Wall -g
TARGET = v6fs
SRC = main.c disk.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

