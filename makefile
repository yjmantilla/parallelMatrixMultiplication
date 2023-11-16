# Makefile for Matrix Multiplication Project

# Compiler settings
CC = gcc
CFLAGS = -g -Wall
LIBS = -lpthread

# Project files
SOURCES = main.c matrix_operations.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = myprogram

# Default target
all: $(EXECUTABLE)

# Rule to create executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Rule to create object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Phony targets
.PHONY: all clean
