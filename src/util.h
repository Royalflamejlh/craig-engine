#ifndef UTIL
#define UTIL

#define ALL_CASTLE 0xF0
#define WHITE_CASTLE_LONG 0x80
#define WHITE_CASTLE_SHORT 0x40
#define BLACK_CASTLE_LONG 0x20
#define BLACK_CASTLE_SHORT 0x10

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#include "types.h"

char* trimWhitespace(char* str);
void moveStrToStruct(char* prev, struct Move* move);
void moveStructToStr(struct Move* move, char* result);
char updateCastling(char castle, struct Move move);
char getColor(char piece);
char opposite(char color);
#endif
