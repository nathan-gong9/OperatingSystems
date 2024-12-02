# Parent Makefile for all parts of the bank simulation project
CC = gcc
CFLAGS = -Wall -pthread

# Directories for each part
PART1_DIR = part1
PART2_DIR = part2
PART3_DIR = part3

# Executables for each part
PART1_EXEC = $(PART1_DIR)/bank
PART2_EXEC = $(PART2_DIR)/bank
PART3_EXEC = $(PART3_DIR)/bank

# Input files
PART1_INPUT = $(PART1_DIR)/input-1.txt
PART2_INPUT = $(PART2_DIR)/input-1.txt
PART3_INPUT = $(PART3_DIR)/input-1.txt

# Output files
PART1_OUTPUT = $(PART1_DIR)/balances.txt
PART2_OUTPUT = $(PART2_DIR)/balances.txt
PART3_OUTPUT = $(PART3_DIR)/balances.txt

# Default target
all: part1 part2 part3

# Rule for part1
part1: $(PART1_EXEC)
	$(PART1_EXEC) $(PART1_INPUT)
	@echo "Part 1 output:"
	@cat $(PART1_OUTPUT)

# Rule for part2
part2: $(PART2_EXEC)
	$(PART2_EXEC) $(PART2_INPUT)
	@echo "Part 2 output:"
	@cat $(PART2_OUTPUT)

# Rule for part3
part3: $(PART3_EXEC)
	$(PART3_EXEC) $(PART3_INPUT)
	@echo "Part 3 output:"
	@cat $(PART3_OUTPUT)

# Compilation rule for part1
$(PART1_EXEC): $(PART1_DIR)/bank.c $(PART1_DIR)/account.h
	$(CC) $(CFLAGS) -o $(PART1_EXEC) $(PART1_DIR)/bank.c

# Compilation rule for part2
$(PART2_EXEC): $(PART2_DIR)/bank.c $(PART2_DIR)/account.h
	$(CC) $(CFLAGS) -o $(PART2_EXEC) $(PART2_DIR)/bank.c

# Compilation rule for part3
$(PART3_EXEC): $(PART3_DIR)/bank.c $(PART3_DIR)/account.h
	$(CC) $(CFLAGS) -o $(PART3_EXEC) $(PART3_DIR)/bank.c

# Clean up compiled files
clean:
	rm -f $(PART1_EXEC) $(PART2_EXEC) $(PART3_EXEC) $(PART1_DIR)/balances.txt $(PART2_DIR)/balances.txt $(PART3_DIR)/balances.txt

# Phony targets (non-file targets)
.PHONY: all part1 part2 part3 clean
