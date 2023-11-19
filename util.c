#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "util.h"

int64_t moveCharToInt(char* prev) {
    if (prev == NULL || strlen(prev) < 4) {
        return 0;
    }

    int64_t result = 0;
    char from_x = prev[0] - 'a';
    char from_y = prev[1] - '1';
    char to_x   = prev[2] - 'a';
    char to_y   = prev[3] - '1';

    result = from_x;
    result = (result << 8) | from_y;
    result = (result << 8) | to_x;
    result = (result << 8) | to_y;

    if (strlen(prev) > 4) {
        char promotion = prev[4];
        result = (result << 8) | promotion;
    }

    printf("\nTurning move %s to the int %lld\r\n", prev, result);

    return result;
}

void moveIntToChar(int64_t move, char* result) {
    for (int i = 0; i < 4; ++i) {
        char c = (char)((move >> (8 * (3 - i))) & 0xFF);
        if (i % 2 == 0) {
            result[i] = c + 'a';
        } else {          
            result[i] = c + '1';
        }
    }

    char promotion = (char)((move >> (8 * 4)) & 0xFF);
    if (promotion != 0) { 
        result[4] = promotion;
        result[5] = '\0'; 
    } else {
        result[4] = '\0';
    }
    printf("\nTurning int %lld into char %s\r\n", move, result);
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
