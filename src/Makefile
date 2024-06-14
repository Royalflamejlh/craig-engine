##
# Compilation Flags
##
L_CC = clang
W_CC = x86_64-w64-mingw32-gcc
SRC = *.c bitboard/*.c tests/*.c
NAME = craig

WRN_FLAGS = -std=gnu11 -Wall -Wextra -Wshadow -Winline -Wno-format-security
SAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=nullability -fsanitize=integer

DFLAGS = -O0 $(WRN_FLAGS) -g -DVERBOSE -DDEBUG
RFLAGS = -O3 $(WRN_FLAGS) -Ofast -funroll-loops -flto -finline-functions -fomit-frame-pointer -march=native

LIBS += -lm -lpthread
L_LIBS += $(LIBS)
W_LIBS += $(LIBS) -static-libgcc -static-libstdc++

##
# Build Targets
##
debug:
	$(L_CC) $(DFLAGS) $(SAN_FLAGS) $(SRC) $(L_LIBS) -o $(NAME)-debug.engine
	$(W_CC) $(DFLAGS) $(SRC) $(W_LIBS) -o $(NAME)-debug.exe

release:
	$(L_CC) $(RFLAGS) $(SRC) $(L_LIBS) -o $(NAME).engine
	$(W_CC) $(RFLAGS) $(SRC) $(W_LIBS) -o $(NAME).exe

clean:
	rm *.engine *.exe