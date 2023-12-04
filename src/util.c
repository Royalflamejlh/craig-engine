#include "util.h"
#include <stdio.h>

void printMove(Move move){
    int from = move & MOVE_FROM_MASK;
    int to = (move & MOVE_TO_MASK) >> 6;
    int promotion = (move & MOVE_PROMOTION_MASK) >> 12;
    int isEnPassant = (move & MOVE_ENPASSANT_MASK) != 0;
    int isCastle = (move & MOVE_CASTLE_MASK) != 0;

    printf("Move: %d to %d", from, to);

    if (promotion) {
        printf(", Promote to ");
        switch (promotion) {
            case PROMOTE_QUEEN:
                printf("Queen");
                break;
            case PROMOTE_BISHOP:
                printf("Bishop");
                break;
            case PROMOTE_KNIGHT:
                printf("Knight");
                break;
            case PROMOTE_ROOK:
                printf("Rook");
                break;
        }
    }

    if (isEnPassant) {
        printf(", En Passant");
    }

    if (isCastle) {
        printf(", Castle");
    }

    printf("\n");
}
