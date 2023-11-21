#ifndef UTIL
#define UTIL

#define ALL_CASTLE 0xF0
#define WHITE_CASTLE_LONG 0x80
#define WHITE_CASTLE_SHORT 0x40
#define BLACK_CASTLE_LONG 0x20
#define BLACK_CASTLE_SHORT 0x10

struct Move {
    unsigned char from_x;
    unsigned char from_y;
    unsigned char to_x;
    unsigned char to_y;
    char promotion;
};


char* trimWhitespace(char* str);
void moveStrToStruct(char* prev, struct Move* move);
void moveStructToStr(struct Move* move, char* result);
char updateCastling(char castle, struct Move move);
char getColor(char piece);
#endif