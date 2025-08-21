CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
LDFLAGS = -lpthread -lrt
TARGET = margolis
SOURCE = margolis.c

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -O2 -o $(TARGET) $(SOURCE) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
