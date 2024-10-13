#include "util.h"
#include "bitboard/bbutils.h"
#include "movement.h"
#include "types.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef PYTHON
static i32 fifo_fd_in, fifo_fd_out;
static pid_t pid;
#endif

void printMove(Move move){
    i32 from = GET_FROM(move);
    i32 to = GET_TO(move);

    i32 rank_from = from / 8;
    i32 file_from = from % 8;

    i32 rank_to = to / 8;
    i32 file_to = to % 8;

    char file_char_from = 'a' + file_from;
    char file_char_to = 'a' + file_to;

    i32 rank_num_from = rank_from + 1;
    i32 rank_num_to = rank_to + 1;

    printf("%c%d%c%d", file_char_from, rank_num_from, file_char_to, rank_num_to);

    switch(GET_FLAGS(move)){
        case DOUBLE_PAWN_PUSH:
            printf(" (DPP)");
            break;
        case KING_CASTLE:
            printf(" (KCl)");
            break;
        case QUEEN_CASTLE:
            printf(" (QCl)");
            break;
        case CAPTURE:
            printf(" (C)");
            break;
        case EP_CAPTURE:
            printf(" (EpC)");
            break;
        case KNIGHT_PROMOTION:
            printf(" (NP)");
            break;
        case BISHOP_PROMOTION:
            printf(" (BP)");
            break;
        case ROOK_PROMOTION:
            printf(" (RP)");
            break;
        case QUEEN_PROMOTION:
            printf(" (QP)");
            break;
        case KNIGHT_PROMO_CAPTURE:
            printf(" (NPC)");
            break;
        case BISHOP_PROMO_CAPTURE:
            printf(" (BPC)");
            break;
        case ROOK_PROMO_CAPTURE:
            printf(" (RPC)");
            break;
        case QUEEN_PROMO_CAPTURE:
            printf(" (QPC)");
            break;
        case QUIET:
        default:
            break;
    }
}

/*
 * Prints the provided move as the best move in the UCI format
 */
void printBestMove(Move move){
    char str[6];
    str[0] = (GET_FROM(move) % 8) + 'a';
    str[1] = (GET_FROM(move) / 8) + '1';
    str[2] = (GET_TO(move) % 8) + 'a';
    str[3] = (GET_TO(move) / 8) + '1';
    str[4] = '\0';

    switch(GET_FLAGS(move)){
        case QUEEN_PROMO_CAPTURE:
        case QUEEN_PROMOTION:
            str[4] = 'q';
            str[5] = '\0';
            break;
        case ROOK_PROMO_CAPTURE:
        case ROOK_PROMOTION:
            str[4] = 'r';
            str[5] = '\0';
            break;
        case BISHOP_PROMO_CAPTURE:
        case BISHOP_PROMOTION:
            str[4] = 'b';
            str[5] = '\0';
            break;
        case KNIGHT_PROMO_CAPTURE:
        case KNIGHT_PROMOTION:
            str[4] = 'n';
            str[5] = '\0';
            break;
        default:
            break;
    }

    #if defined(_WIN32) || defined(_WIN64)
    printf("bestmove %s\r\n", str);
    #else
    printf("bestmove %s\n", str);
    #endif

    fflush(stdout);
    return;
}

void printMoveShort(Move move){
    i32 from = GET_FROM(move);
    i32 to = GET_TO(move);

    i32 rank_from = from / 8;
    i32 file_from = from % 8;

    i32 rank_to = to / 8;
    i32 file_to = to % 8;

    char file_char_from = 'a' + file_from;
    char file_char_to = 'a' + file_to;

    i32 rank_num_from = rank_from + 1;
    i32 rank_num_to = rank_to + 1;

    printf("%c%d%c%d", file_char_from, rank_num_from, file_char_to, rank_num_to);
}

void printMoveSpaced(Move move){
    printMove(move);
    switch(GET_FLAGS(move)){
        case CAPTURE:
            printf("  ");
            break;
        case KNIGHT_PROMOTION:
        case BISHOP_PROMOTION:
        case ROOK_PROMOTION:
        case QUEEN_PROMOTION:
            printf(" ");
            break;
        case QUIET:
            printf("      ");
        default:
            break;
    }
}

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Extracts the move(s) after the 'bm' operation in an EPD line.
 * Returns a dynamically allocated string containing the move(s).
 * The caller is responsible for freeing the allocated memory.
 */
char* get_move_from_epd_line(const char* line) {
    const char* bm_pos = strstr(line, " bm ");
    if (!bm_pos) {
        bm_pos = strstr(line, "bm ");
        if (!bm_pos) {
            return NULL;
        }
    }
    bm_pos += 3;
    while (isspace((unsigned char)*bm_pos)) {
        bm_pos++;
    }
    const char* semicolon_pos = strchr(bm_pos, ';');
    if (!semicolon_pos) {
        semicolon_pos = line + strlen(line);
    }
    size_t move_len = semicolon_pos - bm_pos;

    char* move_str = (char*)malloc(move_len + 1);
    if (!move_str) {
        return NULL;
    }
    strncpy(move_str, bm_pos, move_len);
    move_str[move_len] = '\0';

    size_t end = move_len;
    while (end > 0 && isspace((unsigned char)move_str[end - 1])) {
        move_str[end - 1] = '\0';
        end--;
    }
    return move_str;
}


/**
 * Gets a move from an algebraic notation string, using a list of legal moves.
 */
Move move_from_str_alg(char* str, Position *pos){
    i32 len = strlen(str);
    printf("Parsing move string: %s (length: %d)\n", str, len);
    i32 idx = 0;

    Move move_list[MAX_MOVES] = {0};
    i32 move_count = generateLegalMoves(pos, move_list);
    printf("Generated %d legal moves.\n", move_count);

    char piece_char = 'P'; // Default to pawn
    i32 from_file = -1;    // 0-7
    i32 from_rank = -1;    // 0-7
    i32 to_file = -1;      // 0-7
    i32 to_rank = -1;      // 0-7
    u8 is_capture = 0;
    char promotion_piece = '\0'; // 'N', 'B', 'R', 'Q'
    u8 is_castling = 0;
    i32 castling_type = 0;   // 1 for O-O, 2 for O-O-O

    // Handle castling
    if (strcmp(str, "O-O") == 0 || strcmp(str, "0-0") == 0) {
        is_castling = 1;
        castling_type = 1;
        printf("Detected castling move: O-O (kingside)\n");
    } else if (strcmp(str, "O-O-O") == 0 || strcmp(str, "0-0-0") == 0) {
        is_castling = 1;
        castling_type = 2;
        printf("Detected castling move: O-O-O (queenside)\n");
    }

    if (is_castling) {
        printf("Searching for castling move in move list.\n");
        for (i32 i = 0; i < move_count; i++) {
            Move m = move_list[i];
            u16 flags = GET_FLAGS(m);
            printf("Checking move %d: flags=%d\n", i, flags);
            if ((castling_type == 1 && flags == KING_CASTLE) ||
                (castling_type == 2 && flags == QUEEN_CASTLE)) {
                printf("Found castling move at index %d.\n", i);
                return m;
            }
        }
        printf("Castling move not found.\n");
        return 0;
    }

    // Parse piece character
    if (isupper(str[idx]) && strchr("NBRQK", str[idx])) {
        piece_char = str[idx];
        idx++;
        printf("Detected piece: %c\n", piece_char);
    } else {
        piece_char = 'P'; // Pawn move
        printf("Detected pawn move.\n");
    }

    // Parse disambiguation (before 'x' or destination square)
    while (idx < len && ( (str[idx] >= 'a' && str[idx] <= 'h') || (str[idx] >= '1' && str[idx] <= '8') )) {
        if (str[idx] >= 'a' && str[idx] <= 'h') {
            from_file = str[idx] - 'a';
            printf("Detected disambiguation from file: %c\n", str[idx]);
        } else if (str[idx] >= '1' && str[idx] <= '8') {
            from_rank = str[idx] - '1';
            printf("Detected disambiguation from rank: %c\n", str[idx]);
        }
        idx++;
    }

    // Check for capture indicator
    if (idx < len && str[idx] == 'x') {
        is_capture = 1;
        printf("Detected capture move.\n");
        idx++;
    }

    // Get destination square
    if (idx + 1 <= len && str[idx] >= 'a' && str[idx] <= 'h' && str[idx+1] >= '1' && str[idx+1] <= '8') {
        to_file = str[idx] - 'a';
        to_rank = str[idx+1] - '1';
        printf("Destination square: %c%c (file %d, rank %d)\n", str[idx], str[idx+1], to_file, to_rank);
        idx += 2;
    } else {
        printf("Invalid destination square in move string.\n");
        return 0;
    }

    // Handle promotion
    if (idx < len && (str[idx] == '=' || str[idx] == '(')) {
        idx++; // Skip '=' or '('
        if (idx < len && strchr("NBRQ", toupper(str[idx]))) {
            promotion_piece = toupper(str[idx]);
            printf("Detected promotion to: %c\n", promotion_piece);
            idx++;
        }
        if (idx < len && str[idx] == ')') idx++; // Skip closing ')'
    } else if (idx < len && strchr("NBRQ", toupper(str[idx]))) {
        // Some notations skip '=', e.g., 'e8Q'
        promotion_piece = toupper(str[idx]);
        printf("Detected promotion to: %c\n", promotion_piece);
        idx++;
    }

    // Skip any trailing '+' or '#' symbols
    while (idx < len && (str[idx] == '+' || str[idx] == '#')) {
        idx++;
    }

    // Print parsed move components
    printf("Parsed move components:\n");
    printf("  Piece: %c\n", piece_char);
    if (from_file != -1) printf("  From file: %c\n", 'a' + from_file);
    if (from_rank != -1) printf("  From rank: %d\n", from_rank + 1);
    printf("  To square: %c%d\n", 'a' + to_file, to_rank + 1);
    if (is_capture) printf("  Is capture: Yes\n");
    if (promotion_piece) printf("  Promotion piece: %c\n", promotion_piece);

    // Now, loop through move_list and try to find a move that matches the components
    for (i32 i = 0; i < move_count; i++) {
        Move m = move_list[i];
        i32 from_square = GET_FROM(m);
        i32 to_square = GET_TO(m);
        u16 flags = GET_FLAGS(m);

        // Get moving piece from position
        char moving_piece = pos->charBoard[from_square];
        printf("Checking move %d: from %d (%c%d), to %d (%c%d), piece %c, flags %d\n", 
               i, from_square, 'a' + (from_square % 8), (from_square / 8) + 1, 
               to_square, 'a' + (to_square % 8), (to_square / 8) + 1,
               moving_piece, flags);

        if (toupper(moving_piece) != piece_char) {
            printf("  Skipping move: piece type does not match (%c != %c)\n", toupper(moving_piece), piece_char);
            continue;
        }

        i32 move_to_file = to_square % 8;
        i32 move_to_rank = to_square / 8;
        if (move_to_file != to_file || move_to_rank != to_rank) {
            printf("  Skipping move: destination square does not match\n");
            continue;
        }

        u8 is_move_capture = (flags == CAPTURE || flags == EP_CAPTURE ||
                               flags == KNIGHT_PROMO_CAPTURE || flags == BISHOP_PROMO_CAPTURE ||
                               flags == ROOK_PROMO_CAPTURE || flags == QUEEN_PROMO_CAPTURE);

        if (is_capture != is_move_capture) {
            printf("  Skipping move: capture status does not match\n");
            continue;
        }

        if (promotion_piece) {
            switch (promotion_piece) {
                case 'N':
                    if (!(flags == KNIGHT_PROMOTION || flags == KNIGHT_PROMO_CAPTURE)) {
                        printf("  Skipping move: promotion piece does not match (expected N)\n");
                        continue;
                    }
                    break;
                case 'B':
                    if (!(flags == BISHOP_PROMOTION || flags == BISHOP_PROMO_CAPTURE)) {
                        printf("  Skipping move: promotion piece does not match (expected B)\n");
                        continue;
                    }
                    break;
                case 'R':
                    if (!(flags == ROOK_PROMOTION || flags == ROOK_PROMO_CAPTURE)) {
                        printf("  Skipping move: promotion piece does not match (expected R)\n");
                        continue;
                    }
                    break;
                case 'Q':
                    if (!(flags == QUEEN_PROMOTION || flags == QUEEN_PROMO_CAPTURE)) {
                        printf("  Skipping move: promotion piece does not match (expected Q)\n");
                        continue;
                    }
                    break;
                default:
                    printf("  Skipping move: invalid promotion piece\n");
                    continue;
            }
        } else {
            if (flags == KNIGHT_PROMOTION || flags == BISHOP_PROMOTION ||
                flags == ROOK_PROMOTION || flags == QUEEN_PROMOTION ||
                flags == KNIGHT_PROMO_CAPTURE || flags == BISHOP_PROMO_CAPTURE ||
                flags == ROOK_PROMO_CAPTURE || flags == QUEEN_PROMO_CAPTURE) {
                printf("  Skipping move: move is a promotion but none expected\n");
                continue;
            }
        }

        i32 move_from_file = from_square % 8;
        i32 move_from_rank = from_square / 8;
        if (from_file != -1 && move_from_file != from_file) {
            printf("  Skipping move: from file does not match\n");
            continue;
        }
        if (from_rank != -1 && move_from_rank != from_rank) {
            printf("  Skipping move: from rank does not match\n");
            continue;
        }

        printf("Found matching move at index %d\n", i);
        return m;
    }
    printf("No matching move found.\n");
    return NO_MOVE;
}

u64 perft(ThreadData *td, i32 depth, u8 print){
  Move move_list[MAX_MOVES];
  i32 n_moves, i;
  u64 nodes = 0;

  n_moves = generateLegalMoves(&td->pos, move_list);

  if (depth <= 1) 
    return n_moves;

  for (i = 0; i < n_moves; i++) {
    #ifdef DEBUG
    Position prev_pos = td->pos;
    i32 prev_hash_cur_idx = td->hash_stack.cur_idx;
    i32 prev_hash_reset_idx = td->hash_stack.reset_idx;
    #endif
    make_move(&td->pos, td, move_list[i]);

    #ifdef PYTHON
    checkMoveCount(pos);
    #endif
    u64 count = perft(td, depth - 1, FALSE);
    if(print){
        printMoveShort(move_list[i]);
        printf(": %" PRIu64 "\n", count);
    }
    unmake_move(&td->pos, td, move_list[i]);

    #ifdef DEBUG
    if(!compare_positions(&td->pos, &prev_pos)){
        printf("Error, unmake move did not properly return the position: ");
        printMove(move_list[i]);
        printf("\n\nCorrect Position:\n");
        printPosition(prev_pos, TRUE);
        printf("\n\nFound Position:\n");
        printPosition(td->pos, TRUE);
        while(1);
    }
    if(prev_hash_cur_idx != td->hash_stack.cur_idx){
        printf("Hash stack cur_idx doesnt match previous hash stack!\n");
        while(1);
    }
    if(prev_hash_reset_idx != td->hash_stack.reset_idx){
        printf("Hash stack cur_idx doesnt match previous hash stack!\n");
        while(1);
    }
    td->pos = prev_pos;
    #endif
    

    
    nodes += count;
  }
  if(print){
    printf("Nodes searched: %" PRIu64 "\n", nodes);
  }
  return nodes;
}

char getPiece(Position pos, i32 square){
    return pos.charBoard[square];
}

Move moveStrToType(Position* pos, char* str){
    Move moveList[MAX_MOVES] = {0};
    i32 size = generateLegalMoves(pos, moveList);

    i32 from  = (str[0] - 'a') + ((str[1] - '1') * 8);
    i32 to = (str[2] - 'a') + ((str[3] - '1') * 8);
    char promo = str[4];

    for(i32 i = 0; i < size; i++){
        i32 fromCur = GET_FROM(moveList[i]);
        i32 toCur = GET_TO(moveList[i]);
        char promoCur;
        switch(GET_FLAGS(moveList[i])){
            case QUEEN_PROMO_CAPTURE:
            case QUEEN_PROMOTION:
                promoCur = 'q';
                break;
            case ROOK_PROMO_CAPTURE:
            case ROOK_PROMOTION:
                promoCur = 'r';
                break;
            case BISHOP_PROMO_CAPTURE:
            case BISHOP_PROMOTION:
                promoCur = 'b';
                break;
            case KNIGHT_PROMO_CAPTURE:
            case KNIGHT_PROMOTION:
                promoCur = 'n';
                break;
            default:
                promoCur = '\0';
                break;
        }
        if(fromCur == from && toCur == to && promo == promoCur){
            return moveList[i];
        }
    }
    printf("info string Warning: move: %s not found.\n", str);
    
    return NO_MOVE;
}

void printPV(Move *pv_array, i32 depth) {
    for (i32 i = 0; i < depth; i++) {
        if(pv_array[i] != NO_MOVE){ 
            if(i != 0) printf(" ");
            printMoveShort(pv_array[i]);
        }
    }
}

void printPVInfo(SearchData data){
    printf("info ");
    printf("depth %d ", data.depth);

    i32 score = data.eval;
    if(abs(score) < CHECKMATE_VALUE - MAX_MOVES){
        printf("score cp %d ", score/10);
    }
    else{
        i32 mate = CHECKMATE_VALUE - abs(score);
        mate = (mate + 1) / 2;
        if(score < 0) mate = -mate;
        printf("score mate %d ", mate);
    }

    printf("time %d ", (int)(data.stats.elap_time * 1000));
    printf("nodes %lld ", (long long)data.stats.node_count);
    printf("nps %lld ", (long long)((double)data.stats.node_count / data.stats.elap_time));
    printf("pv ");
    printPV(data.pv_array, data.depth);
    printf("\n");
    fflush(stdout);
}

Stage calculateStage(Position pos){
    Stage stage = MID_GAME;
    if(pos.fullmove_number < OPN_GAME_MOVES) stage = OPN_GAME; 
    if(count_bits(pos.color[0] | pos.color[1]) <= END_GAME_PIECES) stage = END_GAME;
    return stage;
}

u64 millis(){
    struct timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    return _t.tv_sec*1000 + lround(_t.tv_nsec/1e6);
}

/*
 * Returns the recommended search time
 */
u32 calculate_rec_search_time(u32 wtime, u32 winc, u32 btime, u32 binc, u32 moves_remain, u8 turn){
    u32 time = 0;
    u32 advantage = 0;
    if(wtime == 0 && winc == 0 && btime == 0 && binc == 0) return 0;
    if(turn){
        time = wtime + winc;
        advantage = wtime - btime;
    } else{
        time = btime + binc;
        advantage = btime - wtime;
    }
    if(moves_remain) return (time / moves_remain);
    if(time <= 10) return 1;
    if(time <= 100) return 20;
    if(time <= 1000) return 200;
    if(advantage >  0) return ((time) / 10);
    return (time / 20);
}

/*
 * Returns the maximum search time
 */
u32 calculate_max_search_time(u32 wtime, u32 winc, u32 btime, u32 binc, u32 moves_remain, u8 turn){
    u32 time = 0;
    if(wtime == 0 && winc == 0 && btime == 0 && binc == 0) return 0;
    if(turn){
        time = wtime + winc;
    } else{
        time = btime + binc;
    }
    if(moves_remain) return (time / moves_remain);
    return (time / 5) + 1;
}

/**
 * Returns true if the positions are equal
 */
i8 compare_positions(Position *pos1, Position *pos2) {
    for (i32 i = 0; i < 2; i++) {
        if (pos1->pawn[i] != pos2->pawn[i]) {
            printf("Mismatch at pawn[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->pawn[i], pos2->pawn[i]);
            return FALSE;
        }
        if (pos1->bishop[i] != pos2->bishop[i]) {
            printf("Mismatch at bishop[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->bishop[i], pos2->bishop[i]);
            return FALSE;
        }
        if (pos1->knight[i] != pos2->knight[i]) {
            printf("Mismatch at knight[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->knight[i], pos2->knight[i]);
            return FALSE;
        }
        if (pos1->rook[i] != pos2->rook[i]) {
            printf("Mismatch at rook[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->rook[i], pos2->rook[i]);
            return FALSE;
        }
        if (pos1->queen[i] != pos2->queen[i]) {
            printf("Mismatch at queen[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->queen[i], pos2->queen[i]);
            return FALSE;
        }
        if (pos1->king[i] != pos2->king[i]) {
            printf("Mismatch at king[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->king[i], pos2->king[i]);
            return FALSE;
        }
        if (pos1->attack_mask[i] != pos2->attack_mask[i]) {
            printf("Mismatch at attack_mask[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->attack_mask[i], pos2->attack_mask[i]);
            return FALSE;
        }
        if (pos1->color[i] != pos2->color[i]) {
            printf("Mismatch at color[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->color[i], pos2->color[i]);
            return FALSE;
        }
    }
    if (pos1->en_passant != pos2->en_passant) {
        printf("Mismatch at en_passant: %" PRIu64 " != %" PRIu64 "\n", pos1->en_passant, pos2->en_passant);
        return FALSE;
    }
    if (pos1->flags != pos2->flags) {
        printf("Mismatch at flags: %d != %d\n", pos1->flags, pos2->flags);
        return FALSE;
    }
    if (pos1->pinned != pos2->pinned) {
        printf("Mismatch at pinned: %" PRIu64 " != %" PRIu64 "\n", pos1->pinned, pos2->pinned);
        return FALSE;
    }
    if (pos1->hash != pos2->hash) {
        printf("Mismatch at hash: %" PRIu64 "!= %" PRIu64 "\n", pos1->hash, pos2->hash);
        return FALSE;
    }
    if (pos1->material_eval != pos2->material_eval) {
        printf("Mismatch at material_eval: %d != %d\n", pos1->material_eval, pos2->material_eval);
        return FALSE;
    }
    if (pos1->stage != pos2->stage) {
        printf("Mismatch at stage: %d != %d\n", pos1->stage, pos2->stage);
        return FALSE;
    }
    if (pos1->halfmove_clock != pos2->halfmove_clock) {
        printf("Mismatch at halfmove_clock: %d != %d\n", pos1->halfmove_clock, pos2->halfmove_clock);
        return FALSE;
    }
    if (pos1->fullmove_number != pos2->fullmove_number) {
        printf("Mismatch at fullmove_number: %d != %d\n", pos1->fullmove_number, pos2->fullmove_number);
        return FALSE;
    }
    if (memcmp(pos1->charBoard, pos2->charBoard, 64 * sizeof(char)) != 0) {
        printf("Mismatch at charBoard\n");
        return FALSE;
    }

    return TRUE;
}



#ifdef PYTHON
i32 python_init() {
    system("rm /tmp/chess_fifo_in");
    system("rm /tmp/chess_fifo_out");
    system("mkfifo /tmp/chess_fifo_in");
    system("mkfifo /tmp/chess_fifo_out");

    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        execlp("python3", "python3", "chessmoves.py", NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }

    fifo_fd_in = open("/tmp/chess_fifo_in", O_WRONLY);
    fifo_fd_out = open("/tmp/chess_fifo_out", O_RDONLY);
    if (fifo_fd_in == -1 || fifo_fd_out == -1) {
        perror("Error opening FIFO");
        return -1;
    }

    printf("Python running and connected through FIFO\n");

    return 0;
}


static i32 python_movecount(char* fen) {
    char buffer[256];
    ssize_t bytes_read;
    size_t total_bytes_read = 0;

    write(fifo_fd_in, fen, strlen(fen));
    write(fifo_fd_in, "\n", 1);

    memset(buffer, 0, sizeof(buffer));

    do {
        bytes_read = read(fifo_fd_out, buffer + total_bytes_read, sizeof(buffer) - total_bytes_read - 1);
        if (bytes_read == -1) {
            perror("Error reading from FIFO");
            return -1;
        }
        total_bytes_read += bytes_read;
    } while (bytes_read > 0 && buffer[total_bytes_read - 1] != '\n');

    buffer[total_bytes_read] = '\0';

    i32 move_count = atoi(buffer);
    return move_count;
}

i32 python_close() {
    close(fifo_fd_in);
    close(fifo_fd_out);
    kill(pid, SIGKILL);
    unlink("/tmp/chess_fifo_in");
    unlink("/tmp/chess_fifo_out");
    return 0;
}

i32 checkMoveCount(Position pos){
    Move moveList[MAX_MOVES];
    i32 num_moves = generateLegalMoves(pos, moveList);
    char fen[128];
    PositionToFen(pos, fen);
    i32 correct_num_moves = python_movecount(fen);
    if(num_moves != correct_num_moves){
        printf("Incorrect amount of moves found (%d/%d) at pos:\n", num_moves, correct_num_moves);
        printPosition(pos, TRUE);
        for (i32 i = 0; i < num_moves; i++) {
            printMove(moveList[i]);
        }
        return -1;
    }
    return 0;
}

#endif
