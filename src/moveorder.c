#include "moveorder.h"
#include "evaluator.h"
#include "bitboard/bitboard.h"

/* Material Values for move ordering */
const i32 MovePawnValue   =   1000;
const i32 MoveKnightValue =   3500;
const i32 MoveBishopValue =   3600;
const i32 MoveRookValue   =   5000;
const i32 MoveQueenValue  =  10000;
const i32 MoveKingValue   = 100000;

static const i32 SEEPieceValues[] = {
    [WHITE_PAWN  ] = MovePawnValue,
    [BLACK_PAWN  ] = MovePawnValue,
    [WHITE_KNIGHT] = MoveKnightValue,
    [BLACK_KNIGHT] = MoveKnightValue,
    [WHITE_BISHOP] = MoveBishopValue,
    [BLACK_BISHOP] = MoveBishopValue,
    [WHITE_ROOK  ] = MoveRookValue,
    [BLACK_ROOK  ] = MoveRookValue,
    [WHITE_QUEEN ] = MoveQueenValue,
    [BLACK_QUEEN ] = MoveQueenValue,
    [WHITE_KING  ] = MoveKingValue,
    [BLACK_KING  ] = MoveKingValue
};


/* Sorting bonus for castling moves */
const i32 MoveCastleBonus = 30;


/*
 * Evaluates a move in a position
 */
i32 eval_move(Move move, Position* pos){
    i32 eval = 0;
    
    // Get the piece information from the move and the position.
    Square fr_sq = GET_FROM(move);
    Square to_sq = GET_TO(move);
    char fr_piece = pos->charBoard[fr_sq];
    char to_piece = pos->charBoard[to_sq];
    i32 fr_piece_i = pieceToIndex[(int)fr_piece];
    i32 to_piece_i = pieceToIndex[(int)to_piece];

    #ifdef DEBUG
    if(fr_piece_i >= 12 || to_piece_i >= 12){
        printf("Warning illegal piece found at:");
        printPosition(*pos, TRUE);
        printf("from piece: %d", pos->charBoard[fr_sq]);
        printf(" to piece: %d", pos->charBoard[to_sq]);
    }
    #endif

    // Add on the PST values
    eval += PST[pos->stage][fr_piece_i][to_sq] - PST[pos->stage][fr_piece_i][fr_sq];
    
    // Add on calculated values depending on the flag
    switch(GET_FLAGS(move)){
        // Promotion Capture Moves
        case QUEEN_PROMO_CAPTURE:
            eval += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveQueenValue - MovePawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case ROOK_PROMO_CAPTURE:
            eval += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveRookValue - MovePawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case BISHOP_PROMO_CAPTURE:
            eval += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveBishopValue - MovePawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case KNIGHT_PROMO_CAPTURE:
            eval += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveKnightValue - MovePawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        // Capture Moves
        case EP_CAPTURE:
            eval += see(pos, to_sq, ( WHITE_PAWN + ((pos->flags & TURN_MASK) * 6) ), fr_sq, fr_piece_i);
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case CAPTURE:
            eval += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i);
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        // Promotion Moves
        case QUEEN_PROMOTION:
            eval += (MoveQueenValue - MovePawnValue);    
            break;
        case ROOK_PROMOTION:
            eval += (MoveRookValue - MovePawnValue);    
            break;
        case BISHOP_PROMOTION:
            eval += (MoveBishopValue - MovePawnValue);    
            break;
        case KNIGHT_PROMOTION:
            eval += (MoveKnightValue - MovePawnValue);    
            break;
        // Castle Moves
        case QUEEN_CASTLE:
        case KING_CASTLE:
            eval += MoveCastleBonus;
            break;
        default:
            break;
    }

    return eval;
}

/*
 * Evaluates a list of moves
 * Used in the q-search.
 */
void eval_movelist(Position* pos, Move* moveList, i32* moveVals, i32 size){
    for(i32 i = 0; i < size; i++){
        moveVals[i] = 0;
        Move move = moveList[i];
        Square fr_sq = GET_FROM(move);
        Square to_sq = GET_TO(move);

        i32 fr_piece = (i32)pos->charBoard[fr_sq];
        i32 to_piece = (i32)pos->charBoard[to_sq];

        i32 fr_piece_i = pieceToIndex[fr_piece];
        i32 to_piece_i = pieceToIndex[to_piece];

        switch(GET_FLAGS(move)){
            case QUEEN_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveQueenValue;
                moveVals[i] += PST[pos->stage][to_piece_i][to_sq];
                break;
            case ROOK_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveRookValue;
                moveVals[i] += PST[pos->stage][to_piece_i][to_sq];
                break;
            case BISHOP_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveBishopValue;
                moveVals[i] += PST[pos->stage][to_piece_i][to_sq];
                break;
            case KNIGHT_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + MoveKnightValue;
                moveVals[i] += PST[pos->stage][to_piece_i][to_sq];
                break;
            case EP_CAPTURE:
                moveVals[i] += see(pos, to_sq, ( WHITE_PAWN + ((pos->flags & TURN_MASK) * 6) ), fr_sq, fr_piece_i);
                moveVals[i] += PST[pos->stage][to_piece_i][to_sq];
                break;
            case CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i);
                moveVals[i] += PST[pos->stage][to_piece_i][to_sq];
                break;
            default:
                break;
        }
    }
    return;
}

/* Helper function for the Static Exchange Evaluator */
static u64 least_valuable_attacker(Position* pos, u64 attadef, Turn turn, PieceIndex* piece){
    u64 subset = attadef & pos->pawn[turn]; // Pawn
    if (subset){
        *piece = WHITE_PAWN + 6*turn;
        return subset & -subset;
    }
    subset = attadef & pos->knight[turn]; // Knight
    if (subset){
        *piece = WHITE_KNIGHT + 6*turn;
        return subset & -subset;
    }
    subset = attadef & pos->bishop[turn]; // Bishops
    if (subset){
        *piece = WHITE_BISHOP + 6*turn;
        return subset & -subset;
    }
    subset = attadef & pos->rook[turn]; // Rooks
    if (subset){
        *piece = WHITE_ROOK + 6*turn;
        return subset & -subset;
    }
    subset = attadef & pos->queen[turn]; // Queens
    if (subset){
        *piece = WHITE_QUEEN + 6*turn;
        return subset & -subset;
    }
    subset = attadef & pos->king[turn]; // Kings
    if (subset){
        *piece = WHITE_KING + 6*turn;
        return subset & -subset;
    }
   return 0; // None were found
}

/* 
 * Static Exchange Evaluator 
 * As described on the chess programming wiki
 */
i32 see(Position* pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece){
    i32 gain[32], d = 0;
    Turn turn = pos->flags & TURN_MASK;
    u64 mayXray = pos->pawn[0] | pos->pawn[1] | pos->bishop[0] | pos->bishop[1] | pos->rook[0] | pos->rook[1] | pos->queen[0] | pos->queen[1];
    u64 removed = 0;
    u64 fromSet = 1ULL << frSq;
    u64 attadef = getAttackers(pos, toSq, 0) | getAttackers(pos, toSq, 1);
    gain[d]     = SEEPieceValues[target];
    while (fromSet) {
        d++;
        gain[d]  = SEEPieceValues[aPiece] - gain[d-1];
        attadef ^= fromSet;
        removed |= fromSet;
        if ( fromSet & mayXray ){
            attadef |= getXRayAttackers(pos, toSq, 0, removed) | getXRayAttackers( pos, toSq, 1, removed);
        }
        fromSet  = least_valuable_attacker(pos, attadef, (turn + d) & 1, &aPiece);
    }
    while (--d){
        if(gain[d] >= -gain[d-1]) gain[d-1] = -gain[d];
    }
    return gain[0];
}