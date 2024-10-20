# Define the compiler
CC = gcc

# Define the flags
CFLAGS = -Wall -g

# Define the source files
SRCS = main.c command.c string_parser.c

# Define the object files
OBJS = $(SRCS:.c=.o)

# Define the executable
TARGET = pseudo-shell

# Default rule
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean