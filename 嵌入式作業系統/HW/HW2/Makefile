# Makefile for local build on virtual machine

# Compiler settings
CC := gcc
CFLAGS := -Wall -O2

# Target executable
TARGET := HW2

# Source and object files
SRCS := HW2.c
OBJS := $(SRCS:.c=.o)

# Phony targets
.PHONY: all clean

# Default target: build the executable
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remove build artifacts
clean:
	rm -f $(TARGET) $(OBJS)
