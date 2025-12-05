CC = gcc
CFLAGS = -Wall -O2 -pthread
TARGET = HW3
SRC = HW3.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
