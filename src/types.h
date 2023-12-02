//
//  types.h
//  godengine
//
//  Created by John Howard on 11/21/23.
//

#ifndef types_h
#define types_h
#include <stdint.h>
#include <stddef.h>

struct Move {
    unsigned char from_x;
    unsigned char from_y;
    unsigned char to_x;
    unsigned char to_y;
    char promotion;
};

typedef struct {
    uint64_t pawn;
    uint64_t bishop;
    uint64_t knight;
    uint64_t rook;
    uint64_t queen;
    uint64_t king;
    uint64_t white;
    uint64_t black;
} position;

struct Node2 {
    uint16_t move;
    uint64_t hash;
};


struct Node {
    struct Move move; // the move played
    //TODO: Combine status and castle, as they can take up a full 1 byte.
    unsigned char status; // 0 means already played, 2 means the last played move, 4 means a predicted, 8 means root
    unsigned char castle; // bit encoded 1st: white long 2nd: white short, 3rd: black long, 4th: black short
    char color; //Who played the Move
    char board[8][8]; //The board state after the move
    int rating; //The rating of the move
    size_t parent; // Parent node
    size_t *children; // Children nodes
    int childrenCount; //Children count
};

#endif /* types_h */
