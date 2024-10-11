##
# Compilers and Project Setup
##

L_CC = clang
W_CC = x86_64-w64-mingw32-gcc
SRC  = $(wildcard *.c bitboard/*.c tests/*.c)
EXE  = craig

##
# Libraries
##

LIBS += -lm -lpthread
L_LIBS += $(LIBS)
W_LIBS += $(LIBS) -static-libgcc -static-libstdc++

##
# Object Files
##

L_DOBJS = $(SRC:.c=.ld.o)
L_ROBJS = $(SRC:.c=.lr.o)
L_POBJS = $(SRC:.c=.lp.o)
W_DOBJS = $(SRC:.c=.wd.o)
W_ROBJS = $(SRC:.c=.wr.o)

##
# Compilation Flags
##

WRN_FLAGS = -std=gnu11 -Wall -Wextra -Wshadow -Winline -Wno-format-security
SAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=nullability -fsanitize=integer

DFLAGS = -O0 $(WRN_FLAGS) -g -gdwarf-2 -DVERBOSE -DDEBUG
RFLAGS = -O3 $(WRN_FLAGS) -Ofast -funroll-loops -flto -finline-functions -fomit-frame-pointer -march=native
PFLAGS = -O3 $(WRN_FLAGS) -pg -Ofast -funroll-loops -flto -finline-functions -march=native -D__PROFILE


##
# Build Targets for Linux
##

l_debug: $(L_DOBJS)
	$(L_CC) $(L_DOBJS) $(L_LIBS) $(DFLAGS) $(SAN_FLAGS) -o $(EXE)-debug.engine

l_release: $(L_ROBJS)
	$(L_CC) $(L_ROBJS) $(L_LIBS) $(RFLAGS) -o $(EXE).engine

l_profile: $(L_POBJS)
	$(L_CC) $(L_POBJS) $(L_LIBS) $(PFLAGS) -o $(EXE)-prof.engine

%.ld.o: %.c
	$(L_CC) $(DFLAGS) $(SAN_FLAGS) -c $< -o $@

%.lr.o: %.c
	$(L_CC) $(RFLAGS) -c $< -o $@

%.lp.o: %.c
	$(L_CC) $(PFLAGS) -c $< -o $@
##
# Build Targets for Windows
##

w_debug: $(W_DOBJS)
	$(W_CC) $(W_DOBJS) $(W_LIBS) $(DFLAGS) -o $(EXE)-debug.exe

w_release: $(W_ROBJS)
	$(W_CC) $(W_ROBJS) $(W_LIBS) $(RFLAGS) -o $(EXE).exe

%.wd.o: %.c
	$(W_CC) $(DFLAGS) -c $< -o $@

%.wr.o: %.c
	$(W_CC) $(RFLAGS) -c $< -o $@

##
# General
##
linux: l_debug l_release l_profile

windows: w_debug w_release

debug: l_debug w_debug

profile: l_profile

release: l_release w_release

all: linux windows

clean:
	rm -f *.engine *.exe *.ld.o *.lr.o *.lp.o *.wd.o *.wr.o *.out
	rm -f ./bitboard/*.ld.o ./bitboard/*.lr.o ./bitboard/*.lp.o ./bitboard/*.wd.o ./bitboard/*.wr.o
	rm -f ./tests/*.ld.o ./tests/*.lr.o ./tests/*.lp.o ./tests/*.wd.o ./tests/*.wr.o