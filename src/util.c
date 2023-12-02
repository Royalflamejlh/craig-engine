#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "util.h"

void moveStrToStruct(char* prev, struct Move* move) {
    if (prev == NULL || strlen(prev) < 4) {
        return;
    }
    move->from_x = prev[0] - 'a';
    move->from_y = prev[1] - '1';
    move->to_x   = prev[2] - 'a';
    move->to_y   = prev[3] - '1';
    move->promotion = prev[4];

    return;
}

void moveStructToStr(struct Move* move, char* result) {
    result[0] = move->from_x + 'a';
    result[1] = move->from_y + '1';
    result[2] = move->to_x + 'a';
    result[3] = move->to_y + '1';
    result[4] = move->promotion;
    result[5] = '\0';
    return;
}


char* trimWhitespace(char* str) {
    char* end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) {
        return str;
    }

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

char updateCastling(char castle, struct Move move){
    if(move.from_y == 0){
        if(move.from_x == 4)      castle &= ~(WHITE_CASTLE_LONG | WHITE_CASTLE_SHORT);
        else if(move.from_x == 0) castle &= ~WHITE_CASTLE_LONG;
        else if(move.from_x == 7) castle &= ~WHITE_CASTLE_SHORT;
    }
    else if(move.from_y == 7){
        if(move.from_x == 4)      castle &= ~(BLACK_CASTLE_LONG | BLACK_CASTLE_SHORT);
        else if(move.from_x == 0) castle &= ~BLACK_CASTLE_LONG;
        else if(move.from_x == 7) castle &= ~BLACK_CASTLE_SHORT;
    }
    return castle;
}

char getColor(char piece){
    if(isupper(piece)) return 'W';
    return 'B';
}

char opposite(char color){
    if(color == 'B') return 'W';
    return 'B';
}