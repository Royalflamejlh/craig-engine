#include "evaluator.h"
#include "util.h"

#define KING_VALUE     100000
#define QUEEN_VALUE     10000
#define ROOK_VALUE       5000
#define BISHOP_VALUE     3550
#define KNIGHT_VALUE     3500
#define PAWN_VALUE       1000

#define ATTACK_BONUS       10  //Eval for each square under attack
#define D_ATTACK_BONUS    100  //Eval for each enemy under attack
#define DEFEND_BONUS      200  //Eval for each defended piece
#define CHECK_PEN         100  //Eval lost under check
#define TURN_BONUS          5


#define DOUBLE_PAWN_PEN   200  //Eval lost for double pawns
#define ISO_PAWN_PEN      100  //Eval lost for isolated pawns



int evaluate(Position pos){
    int eval_val = 0;
    int turn = pos.flags & WHITE_TURN;

    if(pos.flags & IN_CHECK) eval_val += turn ? -CHECK_PEN : CHECK_PEN;

    eval_val += turn ? TURN_BONUS : -TURN_BONUS;

    //Pieces
    eval_val += KING_VALUE * count_bits(pos.king[1]);
    eval_val += QUEEN_VALUE * count_bits(pos.queen[1]);
    eval_val += ROOK_VALUE * count_bits(pos.rook[1]);
    eval_val += BISHOP_VALUE * count_bits(pos.bishop[1]);
    eval_val += KNIGHT_VALUE * count_bits(pos.knight[1]);
    eval_val += PAWN_VALUE * count_bits(pos.pawn[1]);

    eval_val -= KING_VALUE * count_bits(pos.king[0]);
    eval_val -= QUEEN_VALUE * count_bits(pos.queen[0]);
    eval_val -= ROOK_VALUE * count_bits(pos.rook[0]);
    eval_val -= BISHOP_VALUE * count_bits(pos.bishop[0]);
    eval_val -= KNIGHT_VALUE * count_bits(pos.knight[0]);
    eval_val -= PAWN_VALUE * count_bits(pos.pawn[0]);

    eval_val += DEFEND_BONUS * count_bits(pos.color[1] &  pos.attack_mask[1]);
    eval_val += ATTACK_BONUS * count_bits(pos.attack_mask[1]);
    eval_val += D_ATTACK_BONUS * count_bits(pos.color[0] &  pos.attack_mask[1]);

    eval_val -= DEFEND_BONUS * count_bits(pos.color[0] &  pos.attack_mask[0]);
    eval_val -= ATTACK_BONUS * count_bits(pos.attack_mask[0]);
    eval_val -= D_ATTACK_BONUS * count_bits(pos.color[1] &  pos.attack_mask[0]);

    return eval_val;
}

