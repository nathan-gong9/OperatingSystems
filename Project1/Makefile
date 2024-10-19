CC = gcc
CFLAGS = -Wall -Wextra -g
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
EXEC = proj1

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: clean