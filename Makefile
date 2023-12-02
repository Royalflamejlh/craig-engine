CC = gcc
WIN_CC = x86_64-w64-mingw32-gcc
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -Wno-format-security
CFLAGS = -Wall -Wextra -g $(ASAN_FLAGS)
WIN_CFLAGS = -Wall -Wextra -g
LDFLAGS += -lpthread $(ASAN_FLAGS)
WIN_LDFLAGS = -static-libgcc -static-libstdc++

BINDIR = bin
OBJDIR = obj
LINUX_TARGET = $(BINDIR)/chess
WIN_TARGET = $(BINDIR)/chess.exe

SRCS := $(shell find src -type f -name '*.c')
OBJS = $(SRCS:src/%.c=$(OBJDIR)/%.o)
WIN_OBJS = $(SRCS:src/%.c=$(OBJDIR)/win_%.o)


create_dirs:
	mkdir -p $(OBJDIR) $(addprefix $(OBJDIR)/, $(dir $(SRCS:src/%=%)))
	mkdir -p $(addprefix $(OBJDIR)/win_, $(dir $(SRCS:src/%=%)))

all: create_dirs linux windows

linux: $(LINUX_TARGET)

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINUX_TARGET): $(BINDIR) $(OBJS)
	$(CC) $(CFLAGS) -o $(LINUX_TARGET) $(OBJS) $(LDFLAGS)

windows: $(WIN_TARGET)

$(WIN_TARGET): $(BINDIR) $(WIN_OBJS)
	$(WIN_CC) $(WIN_CFLAGS) -o $(WIN_TARGET) $(WIN_OBJS) $(WIN_LDFLAGS)

$(OBJDIR)/win_%.o: src/%.c
	$(WIN_CC) $(WIN_CFLAGS) -c $< -o $@

$(BINDIR) $(OBJDIR):
	mkdir -p $@

clean:
	rm -f $(LINUX_TARGET) $(WIN_TARGET)
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)

board.o: board.h
tree.o: tree.h
movement.o: movement.h
util.o: util.h
evaluator.o: evaluator.h
types.o: types.h