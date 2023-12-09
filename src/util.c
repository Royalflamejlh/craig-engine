#include "util.h"
#include "movement.h"
#include "types.h"
#include <stdio.h>


void printMove(Move move){
    int from = GET_FROM(move);
    int to = GET_TO(move);

    int rank_from = from / 8;
    int file_from = from % 8;

    int rank_to = to / 8;
    int file_to = to % 8;

    char file_char_from = 'a' + file_from;
    char file_char_to = 'a' + file_to;

    int rank_num_from = rank_from + 1;
    int rank_num_to = rank_to + 1;

    printf("Move: %c%d to %c%d\n", file_char_from, rank_num_from, file_char_to, rank_num_to);

    switch(GET_FLAGS(move)){
        case DOUBLE_PAWN_PUSH:
            printf(" (Double pawn push)");
            break;
        case KING_CASTLE:
            printf(" (King's castle)");
            break;
        case QUEEN_CASTLE:
            printf(" (Queen's castle)");
            break;
        case CAPTURE:
            printf(" (Capture)");
            break;
        case EP_CAPTURE:
            printf(" (En passant capture)");
            break;
        case KNIGHT_PROMOTION:
            printf(" (Knight promotion)");
            break;
        case BISHOP_PROMOTION:
            printf(" (Bishop promotion)");
            break;
        case ROOK_PROMOTION:
            printf(" (Rook promotion)");
            break;
        case QUEEN_PROMOTION:
            printf(" (Queen promotion)");
            break;
        case KNIGHT_PROMO_CAPTURE:
            printf(" (Knight promotion capture)");
            break;
        case BISHOP_PROMO_CAPTURE:
            printf(" (Bishop promotion capture)");
            break;
        case ROOK_PROMO_CAPTURE:
            printf(" (Rook promotion capture)");
            break;
        case QUEEN_PROMO_CAPTURE:
            printf(" (Queen promotion capture)");
            break;
        case QUIET:
        default:
            break;
    }

    printf("\n");
}




uint64_t perft(int depth, Position pos){
  Move move_list[256];
  int n_moves, i;
  uint64_t nodes = 0;

  if (depth == 0) 
    return 1ULL;


  n_moves = generateLegalMoves(pos, move_list);
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