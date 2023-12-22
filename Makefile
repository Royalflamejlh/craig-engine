##
# Linux Vars
##
CC = gcc
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -Wno-format-security
CFLAGS = -Wall -Wextra 
DEBUG_FLAGS = -g $(ASAN_FLAGS) -Werror -Wfatal-errors -D __COMPILE_DEBUG=1
RELEASE_FLAGS = -O3 -Ofast -funroll-loops -flto -finline-functions -fexpensive-optimizations -fomit-frame-pointer -D __FAST_AS_POOP=1
DEBUG_LDFLAGS += -lpthread $(ASAN_FLAGS) 
RELEASE_LDFLAGS += -lpthread
LINUX_TARGET_DEBUG = $(BINDIR)/chess_db
LINUX_TARGET_RELEASE = $(BINDIR)/chess
OBJS = $(SRCS:src/%.c=$(OBJDIR)/%.o)


##
# Windows Vars
##
WIN_CC = x86_64-w64-mingw32-gcc
WIN_CFLAGS = -Wall -Wextra  -Werror -Wfatal-errors -g
WIN_LDFLAGS = -static-libgcc -static-libstdc++
WIN_TARGET = $(BINDIR)/chess.exe
WIN_OBJS = $(SRCS:src/%.c=$(OBJDIR)/win_%.o)


##
# Global Vars
##
SRCS := $(shell find src -type f -name '*.c')
BINDIR = bin
OBJDIR = obj


##
# Create Global Stuff
##
create_dirs:
	mkdir -p $(OBJDIR) $(addprefix $(OBJDIR)/, $(dir $(SRCS:src/%=%)))
	mkdir -p $(addprefix $(OBJDIR)/win_, $(dir $(SRCS:src/%=%)))

all: linux_debug windows

debug: linux_debug windows

release: linux_release


$(BINDIR) $(OBJDIR):
	mkdir -p $@

##
# Linux Stuff
##
linux_debug: CFLAGS += $(DEBUG_FLAGS)
linux_debug: create_dirs $(LINUX_TARGET_DEBUG)

linux_release: CFLAGS += $(RELEASE_FLAGS)
linux_release: create_dirs $(LINUX_TARGET_RELEASE)

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINUX_TARGET_DEBUG): $(BINDIR) $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_LDFLAGS) -o $(LINUX_TARGET_DEBUG) $(OBJS)

$(LINUX_TARGET_RELEASE): $(BINDIR) $(OBJS)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o $(LINUX_TARGET_RELEASE) $(OBJS)


##
# Windows Stuff
##
windows: create_dirs $(WIN_TARGET)

$(OBJDIR)/win_%.o: src/%.c
	$(WIN_CC) $(WIN_CFLAGS) -c $< -o $@

$(WIN_TARGET): $(BINDIR) $(WIN_OBJS)
	$(WIN_CC) $(WIN_CFLAGS) -o $(WIN_TARGET) $(WIN_OBJS) $(WIN_LDFLAGS)


##
# Clean Up
##
clean:
	rm -f $(LINUX_TARGET) $(WIN_TARGET)
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)

clean_obj:
	rm -rf $(OBJDIR)