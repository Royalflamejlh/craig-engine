#ifndef UTIL_H
#define UTIL_H
#include <stdlib.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef DEBUG
#include <stdio.h>
#define DEBUG_PRINT(x) printf(x)
#else
#define DEBUG_PRINT(x) do {} while (0)
#endif


#include "types.h"
void printMove(Move move);
void printBestMove(Move move);
void printMoveShort(Move move);
void printMoveSpaced(Move move);
u64 perft(ThreadData *td, i32 depth, u8 print);
i32 checkMoveCount(Position pos);
i32 python_init();
i32 python_close();

Move moveStrToType(Position* pos, char* str);
Stage calculateStage(Position pos);
u32 calculate_rec_search_time(u32 wtime, u32 winc, u32 btime, u32 binc, u32 moves_remain, u8 turn);
u32 calculate_max_search_time(u32 wtime, u32 winc, u32 btime, u32 binc, u32 moves_remain, u8 turn);

i8 compare_positions(Position *pos1, Position *pos2);

void printPV(Move *pv_array, i32 depth);
void printPVInfo(SearchData data);

u64 millis();

static inline i32 getlsb(uint64_t bb) {
    return __builtin_ctzll(bb);
}

// Returns 1 if the given square is a light color square
// 0 if a dark color square
static inline u8 is_square_light(Square sq){
    return ((sq / 8) + (sq % 8)) % 2 == 0;
}

static inline i32 count_bits(u64 v){
    u32 c;
    for (c = 0; v; c++){
        v &= v - 1;
    }
    return c;
}

static inline u64 random_uint64() {
  u64 u1, u2, u3, u4;
  u1 = (u64)(rand()) & 0xFFFF; u2 = (u64)(rand()) & 0xFFFF;
  u3 = (u64)(rand()) & 0xFFFF; u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

static inline void movcpy (Move* pTarget, const Move* pSource, i32 n) {
   while (n-- && (*pTarget++ = *pSource++));
}

/*
 * Returns whether or not their is a pawn is in a position that can promote (on the row only)
 */
static inline u8 canPromotePawn(Position *pos){
   u8 turn = pos->flags & WHITE_TURN;
   u64 row = turn ? 0x00FF000000000000ULL : 0x000000000000FF00ULL;
   return (pos->pawn[turn] & row) != 0;
}

/*
* Returns whether the position has Insufficient material / Drawn
*/
static inline u8 isInsufficient(Position* pos){
   if(pos->stage != END_GAME) return FALSE;
   u32 piece_count_w = count_bits(pos->color[0]);
   u32 piece_count_b = count_bits(pos->color[1]);
   if(piece_count_w == 1 && piece_count_b == 1) return TRUE;
   if(piece_count_w <= 2 && piece_count_b == 1){
      if((pos->knight[0] | pos->bishop[0])) return TRUE; 
   }
   if(piece_count_w == 1 && piece_count_b <= 2){
      if((pos->knight[1] | pos->bishop[1])) return TRUE;
   }
   if(piece_count_w <= 2 && piece_count_b <= 2){
      if((pos->knight[0] | pos->bishop[0]) && (pos->knight[1] | pos->bishop[1])) return TRUE;
   }
   return FALSE;
}

/*
* Returns whether the position is a Repetition
*/
static inline i32 isRepetition(ThreadData *td){
   for(i32 i = td->hash_stack.reset_idx; i != td->hash_stack.cur_idx; i = (i + 1) % HASHSTACK_SIZE){
      if(td->hash_stack.hash[i] == td->hash_stack.hash[td->hash_stack.cur_idx]) return 1;
   }
   return 0;
}

#endif
