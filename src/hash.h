#pragma once
#include "types.h"
#ifdef DEBUG
void debug_hash_difference(u64 hash1, u64 hash2);
#endif

u64 hashPosition(Position *pos);
u64 hash_update_piece(u64 hash, i32 sq, i32 piece);
u64 hash_update_turn(u64 hash);
u64 hash_update_enpassant(u64 hash, i32 sq);
u64 hash_update_castle(u64 hash, PositionFlag castle);
void initZobrist(void);
