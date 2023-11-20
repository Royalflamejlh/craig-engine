CC = gcc
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -Wno-format-security
CFLAGS = -Wall -Wextra -g $(ASAN_FLAGS)
LDFLAGS += -lpthread $(ASAN_FLAGS)

TARGET = chess

SRCS = main.c board.c tree.c movement.c util.c evaluator.c

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS)

board.o: board.h
tree.o: tree.h
movement.o: movement.h
util.o: util.h
evaluator.o: evaluator.h
