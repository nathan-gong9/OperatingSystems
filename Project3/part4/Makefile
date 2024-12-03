# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2

# Target executable
TARGET = bank

# Source files
SOURCES = bank.c
HEADERS = account.h

# Build rule for the executable
$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

# Clean up build artifacts
clean:
	rm -f $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET) input.txt

.PHONY: clean run
