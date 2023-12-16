#include "evaluator.h"

#define KING_VALUE     100 000
#define QUEEN_VALUE     10 000
#define ROOK_VALUE       5 000
#define BISHOP_VALUE     3 550
#define KNIGHT_VALUE     3 500
#define PAWN_VALUE       1 000

#define ATTACK_BONUS        10  //Elo for each square under attack
#define DEFEND_BONUS       200  //Elo for each defended piece
#define CHECK_PEN          100  //Elo lost under check

#define DOUBLE_PAWN_PEN    200  //Elo lost for double pawns
#define ISO_PAWN_PEN       100  //Elo lost for isolated pawns



int evaluate(Position pos){
    int eval_val = 0;
    int turn = pos.flags & WHITE_TURN;

    if(pos.flags & IN_CHECK) eval_val -= CHECK_PEN;

    //Pieces
    for(int i = 0; i < 64; i++){
        
    }

    return turn ? eval_val : -eval_val;
}