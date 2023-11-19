
CC = gcc
CFLAGS = -Wall -Wextra -g


TARGET = chess


SRCS = main.c board.c tree.c movement.c util.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f $(TARGET) $(OBJS)

board.o: board.h
tree.o: tree.h
movement.o: movement.h
util.o: util.h
