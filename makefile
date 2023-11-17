# Compiler settings
CC = gcc
CFLAGS = -Wall
SHARED_FLAGS = -fPIC -shared

# Target executables and library
TARGET1 = matmulseq_file
TARGET2 = verify
LIB = libaux_matrix_operations.so

# Object files
AUX_OBJS = aux_matrix_operations.o
OBJS1 = matmulseq_file.o $(AUX_OBJS)
OBJS2 = verify.o $(AUX_OBJS)

# Source and header files for the shared library
SRC = aux_matrix_operations.c
HEADER = aux_matrix_operations.h

# Default target
all: $(TARGET1) $(TARGET2) $(LIB)

# Compile each main file and the shared code
$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $(OBJS1)

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(OBJS2)

# Rule to create the shared library
$(LIB): $(SRC) $(HEADER)
	$(CC) $(SHARED_FLAGS) -o $@ $(SRC)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGET1) $(TARGET2) $(LIB) *.o

# Phony targets
.PHONY: all clean
