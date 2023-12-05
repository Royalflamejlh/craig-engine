#include "util.h"
#include "movement.h"
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




uint64_t perft(int depth, Position pos){
  Move move_list[256];
  int n_moves, i;
  uint64_t nodes = 0;

  if (depth == 0) 
    return 1ULL;

  
  generateLegalMoves(pos, move_list, &n_moves);
  for (i = 0; i < n_moves; i++) {
    //MakeMove(move_list[i]);
    nodes += perft(depth - 1, pos);
    //UndoMove(move_list[i]);
  }
  return nodes;
}

char getPiece(Position pos, int square){
    return pos.charBoard[square];
}