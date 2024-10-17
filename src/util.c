#include "util.h"
#include "bitboard/bbutils.h"
#include "movement.h"
#include "types.h"
#include "params.h"
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
    i32 idx = 0;

    Move move_list[MAX_MOVES] = {0};
    i32 move_count = generateLegalMoves(pos, move_list);

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
    } else if (strcmp(str, "O-O-O") == 0 || strcmp(str, "0-0-0") == 0) {
        is_castling = 1;
        castling_type = 2;
    }

    if (is_castling) {
        for (i32 i = 0; i < move_count; i++) {
            Move m = move_list[i];
            u16 flags = GET_FLAGS(m);
            if ((castling_type == 1 && flags == KING_CASTLE) ||
                (castling_type == 2 && flags == QUEEN_CASTLE)) {
                return m;
            }
        }
        return 0;
    }

    // Parse piece character
    if (isupper(str[idx]) && strchr("NBRQK", str[idx])) {
        piece_char = str[idx];
        idx++;
    } else {
        piece_char = 'P'; // Pawn move
    }

    // Disambiguation parsing
    int disambiguation_parsed = 0;
    while (disambiguation_parsed < 2 && idx < len) {
        // Check for capture indicator
        if (str[idx] == 'x') {
            break; // Stop disambiguation parsing
        }
        // Check if next two characters are destination square
        if (idx +1 < len && str[idx] >= 'a' && str[idx] <= 'h' && str[idx+1] >= '1' && str[idx+1] <= '8') {
            // Destination square detected
            break;
        }
        if (str[idx] >= 'a' && str[idx] <= 'h' && from_file == -1) {
            from_file = str[idx] - 'a';
            idx++;
            disambiguation_parsed++;
        } else if (str[idx] >= '1' && str[idx] <= '8' && from_rank == -1) {
            from_rank = str[idx] - '1';
            idx++;
            disambiguation_parsed++;
        } else {
            // Unrecognized character
            break;
        }
    }

    // Check for capture indicator
    if (idx < len && str[idx] == 'x') {
        is_capture = 1;
        idx++;
    }

    // Parse destination square
    if (idx +1 < len && str[idx] >= 'a' && str[idx] <= 'h' && str[idx+1] >= '1' && str[idx+1] <= '8') {
        to_file = str[idx] - 'a';
        to_rank = str[idx+1] - '1';
        idx += 2;
    } else {
        return 0;
    }

    // Handle promotion
    if (idx < len && (str[idx] == '=' || str[idx] == '(')) {
        idx++;
        if (idx < len && strchr("NBRQ", toupper(str[idx]))) {
            promotion_piece = toupper(str[idx]);
            idx++;
        }
        if (idx < len && str[idx] == ')') idx++; // Skip closing ')'
    } else if (idx < len && strchr("NBRQ", toupper(str[idx]))) {
        promotion_piece = toupper(str[idx]);
        idx++;
    }

    while (idx < len && (str[idx] == '+' || str[idx] == '#')) {
        idx++;
    }

    for (i32 i = 0; i < move_count; i++) {
        Move m = move_list[i];
        i32 from_square = GET_FROM(m);
        i32 to_square = GET_TO(m);
        u16 flags = GET_FLAGS(m);
        char moving_piece = pos->charBoard[from_square];
        if (toupper(moving_piece) != piece_char) {
            continue;
        }
        i32 move_to_file = to_square % 8;
        i32 move_to_rank = to_square / 8;
        if (move_to_file != to_file || move_to_rank != to_rank) {
            continue;
        }
        u8 is_move_capture = (flags == CAPTURE || flags == EP_CAPTURE ||
                               flags == KNIGHT_PROMO_CAPTURE || flags == BISHOP_PROMO_CAPTURE ||
                               flags == ROOK_PROMO_CAPTURE || flags == QUEEN_PROMO_CAPTURE);
        if (is_capture != is_move_capture) {
            continue;
        }
        if (promotion_piece) {
            switch (promotion_piece) {
                case 'N':
                    if (!(flags == KNIGHT_PROMOTION || flags == KNIGHT_PROMO_CAPTURE)) {
                        continue;
                    }
                    break;
                case 'B':
                    if (!(flags == BISHOP_PROMOTION || flags == BISHOP_PROMO_CAPTURE)) {
                        continue;
                    }
                    break;
                case 'R':
                    if (!(flags == ROOK_PROMOTION || flags == ROOK_PROMO_CAPTURE)) {
                        continue;
                    }
                    break;
                case 'Q':
                    if (!(flags == QUEEN_PROMOTION || flags == QUEEN_PROMO_CAPTURE)) {
                        continue;
                    }
                    break;
                default:
                    continue;
            }
        } else {
            if (flags == KNIGHT_PROMOTION || flags == BISHOP_PROMOTION ||
                flags == ROOK_PROMOTION || flags == QUEEN_PROMOTION ||
                flags == KNIGHT_PROMO_CAPTURE || flags == BISHOP_PROMO_CAPTURE ||
                flags == ROOK_PROMO_CAPTURE || flags == QUEEN_PROMO_CAPTURE) {
                continue;
            }
        }
        i32 move_from_file = from_square % 8;
        i32 move_from_rank = from_square / 8;
        if (from_file != -1 && move_from_file != from_file) {
            continue;
        }
        if (from_rank != -1 && move_from_rank != from_rank) {
            continue;
        }
        return m;
    }
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
    make_move(td, move_list[i]);

    #ifdef PYTHON
    checkMoveCount(pos);
    #endif
    u64 count = perft(td, depth - 1, FALSE);
    if(print){
        printMoveShort(move_list[i]);
        printf(": %" PRIu64 "\n", count);
    }
    unmake_move(td, move_list[i]);

    #ifdef DEBUG
    if(!compare_positions(&td->pos, &prev_pos)){
        printf("Error in perft, unmake move did not properly return the position: ");
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
        printf("Hash stack reset_idx doesnt match previous hash stack!\n");
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

Stage calculateStage(Position *pos){
    Stage stage = MID_GAME;
    if(pos->fullmove_number < OPN_GAME_MOVES) stage = OPN_GAME; 
    if(count_bits(pos->color[0] | pos->color[1]) <= END_GAME_PIECES) stage = END_GAME;
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
    u8 return_value = TRUE;
    for (i32 i = 0; i < 2; i++) {
        if (pos1->pawn[i] != pos2->pawn[i]) {
            printf("Mismatch at pawn[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->pawn[i], pos2->pawn[i]);
            return_value = FALSE;
        }
        if (pos1->bishop[i] != pos2->bishop[i]) {
            printf("Mismatch at bishop[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->bishop[i], pos2->bishop[i]);
            return_value = FALSE;
        }
        if (pos1->knight[i] != pos2->knight[i]) {
            printf("Mismatch at knight[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->knight[i], pos2->knight[i]);
            return FALSE;
        }
        if (pos1->rook[i] != pos2->rook[i]) {
            printf("Mismatch at rook[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->rook[i], pos2->rook[i]);
            return_value = FALSE;
        }
        if (pos1->queen[i] != pos2->queen[i]) {
            printf("Mismatch at queen[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->queen[i], pos2->queen[i]);
            return_value = FALSE;
        }
        if (pos1->king[i] != pos2->king[i]) {
            printf("Mismatch at king[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->king[i], pos2->king[i]);
            return_value = FALSE;
        }
        if (pos1->attack_mask[i] != pos2->attack_mask[i]) {
            printf("Mismatch at attack_mask[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->attack_mask[i], pos2->attack_mask[i]);
            return_value = FALSE;
        }
        if (pos1->color[i] != pos2->color[i]) {
            printf("Mismatch at color[%d]: %" PRIu64 " != %" PRIu64 "\n", i, pos1->color[i], pos2->color[i]);
            return_value = FALSE;
        }
    }
    if (pos1->en_passant != pos2->en_passant) {
        printf("Mismatch at en_passant: %" PRIu64 " != %" PRIu64 "\n", pos1->en_passant, pos2->en_passant);
        return_value = FALSE;
    }
    if (pos1->flags != pos2->flags) {
        printf("Mismatch at flags: %d != %d\n", pos1->flags, pos2->flags);
        return_value = FALSE;
    }
    if (pos1->pinned != pos2->pinned) {
        printf("Mismatch at pinned: %" PRIu64 " != %" PRIu64 "\n", pos1->pinned, pos2->pinned);
        return_value = FALSE;
    }
    if (pos1->hash != pos2->hash) {
        printf("Mismatch at hash: %" PRIu64 "!= %" PRIu64 "\n", pos1->hash, pos2->hash);
        return_value = FALSE;
    }
    if (pos1->material_eval != pos2->material_eval) {
        printf("Mismatch at material_eval: %d != %d\n", pos1->material_eval, pos2->material_eval);
        return_value = FALSE;
    }
    if (pos1->stage != pos2->stage) {
        printf("Mismatch at stage: %d != %d\n", pos1->stage, pos2->stage);
        return_value = FALSE;
    }
    if (pos1->halfmove_clock != pos2->halfmove_clock) {
        printf("Mismatch at halfmove_clock: %d != %d\n", pos1->halfmove_clock, pos2->halfmove_clock);
        return_value = FALSE;
    }
    if (pos1->fullmove_number != pos2->fullmove_number) {
        printf("Mismatch at fullmove_number: %d != %d\n", pos1->fullmove_number, pos2->fullmove_number);
        return_value = FALSE;
    }
    if (memcmp(pos1->charBoard, pos2->charBoard, 64 * sizeof(char)) != 0) {
        printf("Mismatch at charBoard\n");
        return_value = FALSE;
    }

    return return_value;
}

/**
 * Gets a random position with at least 1 move
 */
Position get_random_position(){
    Position pos = fen_to_position(START_FEN);
    i32 num_moves = random_uint64() % 100;
    Move move_list[MAX_MOVES] = {0};
    for (int i = 0; i < num_moves; ++i) {
        i32 move_count = generateLegalMoves(&pos, move_list);
        if (move_count == 0) {
            pos = fen_to_position(START_FEN);
            continue;
        }
        _make_move(&pos, move_list[rand() % move_count]);
    }
    i32 move_count = generateLegalMoves(&pos, move_list);
    if (move_count == 0) pos = fen_to_position(START_FEN); // Return default if no legal moves
    return pos;
}

#ifdef __PROFILE
#include "search.h"
#include "globals.h"

void play_self(){
    ThreadData td = {0};
    td.pos = fen_to_position(START_FEN);
    set_global_position(td.pos);
    Move move_list[MAX_MOVES];
    while(generateLegalMoves(&td.pos, move_list) && td.pos.halfmove_clock < 20){
        SearchParameters sp = {0};
        sp.depth = MAX_DEPTH - 1;
        sp.can_shorten = FALSE;
        start_search(sp);
        sleep(1);
        stopSearch();
        Move move = get_global_best_move();
        printPosition(td.pos, FALSE);
        make_move(&td, move);
        set_global_position(td.pos);
    }
    printf("Finished playing self!");
}
#endif


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
