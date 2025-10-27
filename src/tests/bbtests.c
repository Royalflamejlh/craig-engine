//
//  bbtests.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbtests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../tree.h"
#include "../bitboard/bitboard.h"
#include "../bitboard/magic.h"
#include "../tree.h"
#include "../movement.h"
#include "../util.h"
#include "../hash.h"
#include "../transposition.h"
#include "../evaluator.h"
#include "../globals.h"
#include "../search.h"
#include "../moveorder.h"

// #define SELECT_SORT_TEST
// #define MOVE_GEN_TEST
// #define MOVE_MAKE_TEST
#define PERF_TEST
#define SEE_TEST
#define MOVE_SORT_TEST
// #define PUZZLE_TEST

i32 testBB(void) {
    #ifdef PYTHON
    python_init();
    #endif


    FILE *file;
    char line[1024];
    Position pos;
    Move moveList[MAX_MOVES];
    i32 size, expectedMoves;

    (void)size;
    (void)expectedMoves;
    (void)moveList;
    (void)pos;

    #ifdef MOVE_GEN_TEST
    printf("\n---------------------------------- MOVE GEN TESTING ----------------------------------\n\n");
    

    file = fopen("../perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file ../perftsuite.epd");
        return -1;
    }
    while (fgets(line, sizeof(line), file)) {
        char *fenEnd = strchr(line, ';');
        if (fenEnd) *fenEnd = '\0';
        char *fen = line;
        pos = fen_to_position(fen);
        if (fenEnd) *fenEnd = ';'; 

        char *token;
        while ((token = strtok(fenEnd ? fenEnd + 1 : NULL, " ;")) != NULL) {
            if (token[0] == 'D' && token[1] == '1') {
                expectedMoves = atoi(token + 3);
                size = generateLegalMoves(&pos, moveList);
                if (size != expectedMoves) {
                    printf("Failed to get correct amount of moves for Position %s, correct: %d, found: %d\n", fen, expectedMoves, size);
                    printPosition(pos, TRUE);
                    for (i32 i = 0; i < size; i++) {
                        printMove(moveList[i]);
                    }
                    while(1);
                    return -1;
                }
                break;
            }
        }
    }
    fclose(file);
    printf("Finished Quick Movegen Check \n");
    #endif

    #ifdef MOVE_GEN_TEST
    printf("\n---------------------------------- MOVE MAKING TESTING ----------------------------------\n\n");
    time_t t;
    srand((unsigned) time(&t));

    
    printf("Starting Move Making Check\n");
    Move threatMoveList[MAX_MOVES];
    i32 threatSize;
    for(i32 j = 0; j < 100; j++){
        char* FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        pos = fen_to_position(FEN);
        size = generateLegalMoves(&pos, moveList);
        while(size != 0 && pos.halfmove_clock < 50){
            i32 randMove = rand() % size;
            _make_move(&pos, moveList[randMove]);
            size = generateLegalMoves(&pos, moveList);
            threatSize = generateThreatMoves(&pos, threatMoveList);
            for(i32 k = 0; k < threatSize; k++){
                char found = 0;
                for(i32 l = 0; l < size; l++){
                    if(threatMoveList[k] == moveList[l]) found = 1;
                }
                if(!found){
                    printf("Move in threat moves, that is not in move list!\n");
                    return -1;
                }
            }
        }
    }
    printf("Move Making Check Complete.\n");

    #endif

    #ifdef SELECT_SORT_TEST
    #include "../tables.h"
    printf("\n------------------------------ SELECT SORT TESTING --------------------------------\n\n");
    ThreadData ss_td = {0};
    ss_td.pos = fen_to_position("r2qk2r/pbp2ppp/1pn1pn2/3p4/3P1B2/P1PBPN2/2P2PPP/R2QK2R w KQkq - 1 9");
    u32 evalIdx = 0;
    
    Move ss_move_list[MAX_MOVES] = {0};
    i32  ss_move_vals[MAX_MOVES] = {0};
    u32 ss_list_size = generateLegalMoves(&ss_td.pos, ss_move_list);
    
    storeKillerMove(&ss_td.km, 0, ss_move_list[8]);
    Move ss_tt_move = ss_move_list[9];

    // Testing move vals
    for (u32 i = 0; i < ss_list_size; i++)  {
        printf("move_list[%d] val = %d ; ", i, eval_move(ss_move_list[i], &ss_td.pos));
        printMove(ss_move_list[i]);
        printf("\n");
    }

    for (u32 i = 0; i < ss_list_size; i++)  {
        evalIdx = select_sort(&ss_td, i, evalIdx, ss_move_list, ss_move_vals, ss_list_size, ss_tt_move, 0);
    }
    printf("\n------------------------------------------------------------------------\n");

    for (u32 i = 0; i < ss_list_size; i++)  {
        printf("move_list[%d] val = %d ; ", i, ss_move_vals[i]);
        printMove(ss_move_list[i]);
        printf("\n");
    }
    return

    #endif

    #ifdef PERF_TEST
    printf("\n---------------------------------- PERFT TESTING ----------------------------------\n\n");
    int perft_depth = 5;
    printf("Running perft suite to depth: %d\n", perft_depth);
    file = fopen("../perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    ThreadData temp_td = {0};
    while (fgets(line, sizeof(line), file)) {
        char *fen = line;
        temp_td.pos = fen_to_position(fen);
        for(int depth = 1; depth < perft_depth; depth++){
            i64 num_moves = perft(&temp_td, depth, FALSE);
            i64 real_num_moves = get_perft_fen_depth(fen, depth);
            if(real_num_moves == -1) continue;
            if(num_moves != real_num_moves){
                printf("Incorrect number of moves found for fen: \n %s \n", fen);
                printf("Found %" PRIi64 " moves, expected %" PRIi64 " moves at depth %d \n", num_moves, real_num_moves, depth);
                fflush(stdout);
                while(1);
            }
        }
        printf(".");
        fflush(stdout);
    }

    printf("\nPerft Suite Completed Successfully!\n");

    fclose(file);
    #endif


    #ifdef MOVE_SORT_TEST
    #define NUM_SORT_TESTS 1000
    #define MS_SEARCH_TIME 1
    printf("\n----------------------------- MOVE SORT TESTING ------------------------------\n\n");
    printf("Running %d move sort tests, each with a search time of %d seconds\n", NUM_SORT_TESTS, MS_SEARCH_TIME);

    i32 ms_correct = 0;
    i32 ms_incorrect = 0;

    for(i32 i = 0; i < NUM_SORT_TESTS; i++){
        Position ms_pos = get_random_position();
        Move ms_move_list[MAX_MOVES] = {0};
        i32 ms_move_vals[MAX_MOVES] = {0};
        u16 ms_moves = generateLegalMoves(&ms_pos, ms_move_list);
        eval_movelist(&ms_pos, ms_move_list, ms_move_vals, ms_moves);

        i32 max = INT32_MIN;
        i32 max_idx = 0;
        for(i32 j = 0; j < ms_moves; j++){
            if(ms_move_vals[j] > max){
                max = ms_move_vals[j];
                max_idx = j;
            }
        }
        Move sorted_best_move = ms_move_list[max_idx];

        set_global_position(ms_pos);
        SearchParameters sp = {0};
        sp.depth = MAX_DEPTH - 1;
        sp.can_shorten = FALSE;

        start_search(sp);
        sleep(MS_SEARCH_TIME);
        stopSearch();

        Move found_move = get_global_best_move();
        if (found_move != sorted_best_move){
            printf("x");
            ms_incorrect++;
        }
        else {
            printf(".");
            ms_correct++;
        }
        fflush(stdout);
    }
    printf("\nPercent of Moves Sorted Correctly: %f (%d/%d)\n", (float)ms_correct / ((float)(ms_incorrect + ms_correct)) * 100.f, ms_correct, ms_incorrect + ms_correct);

    printf("\nMove Sorting Test Complete\n");

    #endif


    #ifdef PUZZLE_TEST
    #define PUZZLE_SEARCH_TIME 60
    #define PUZZLE_TEST_VERBOSE
    printf("\n--------------------------------- PUZZLE TESTING ----------------------------------\n\n");

    printf("\nRunning Puzzle test each with a search time of %d seconds\n", PUZZLE_SEARCH_TIME);

    file = fopen("../puzzles/ERET.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    run_get_best_move = TRUE;

    i32 correct = 0;
    i32 incorrect = 0;

    while (fgets(line, sizeof(line), file)) {
        char *fen = line;
        Position puzzle_pos = fen_to_position(fen);
        char* bm_ptr = strstr(fen, " bm ");
        if (bm_ptr == NULL) {
            continue;
        }
        bm_ptr += 4; 
        char* end_ptr = strchr(bm_ptr, ';');
        if (end_ptr == NULL) {
            continue;
        }

        char moves_str[256];

        size_t len = end_ptr - bm_ptr;

        if (len >= sizeof(moves_str)) {
            printf("Moves string too long\n");
            continue;
        }

        strncpy(moves_str, bm_ptr, len);
        moves_str[len] = '\0';

        Move correct_moves[10];

        int num_correct_moves = 0;

        char* token = strtok(moves_str, " ");

        while (token != NULL && num_correct_moves < MAX_MOVES) {
            Move m = move_from_str_alg(token, &puzzle_pos);
            if (m != NO_MOVE) {
                correct_moves[num_correct_moves++] = m;
            } else {
                printf("Move was not found from move string %s\n and fen %s \n", token, fen);
            }
            token = strtok(NULL, " ");
        }

        if (num_correct_moves == 0) {
            continue;
        }

        set_global_position(puzzle_pos);
        SearchParameters sp = {0};
        sp.depth = MAX_DEPTH - 1;
        sp.can_shorten = FALSE;

        start_search(sp);
        sleep(PUZZLE_SEARCH_TIME);
        stopSearch();

        Move found_move = get_global_best_move();

        int found_correct_move = 0;
        for (int i = 0; i < num_correct_moves; i++) {
            if (found_move == correct_moves[i]) {
                found_correct_move = 1;
                break;
            }
        }

        #ifdef PUZZLE_TEST_VERBOSE
        if (!found_correct_move) {
            printf("\n------------- Test Failed -------------\n");
            printf("Fen: %s \nCorrect Moves: ", fen);
            for (int i = 0; i < num_correct_moves; i++) {
                printMove(correct_moves[i]);
                printf(" ");
            }
            printf("\nFound move: ");
            printMove(found_move);
            printf("\n");
            while(1);
        }
        else{
            printf("Puzzle test #%d passed!", correct + incorrect);
        }
        #else
        if (!found_correct_move) printf("x");
        else printf(".");
        #endif

        if (!found_correct_move) incorrect++;
        else correct++;
    }

    printf("\nPercent of Puzzles Correct: %f (%d/%d)\n", (float)correct / ((float)(incorrect + correct)) * 100.f, correct, incorrect + correct);

    printf("\nPuzzle Tests Complete\n");

    fclose(file);

    #endif

    #ifdef SEE_TEST
    printf("\n---------------------------------- SEE TESTING ------------------------------------\n\n");
    Position testpos = fen_to_position("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -");
    i32 val = see(&testpos, E5, BLACK_PAWN, E1, WHITE_ROOK);
    if(val < 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    }

    testpos = fen_to_position("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -");
    val = see(&testpos, E5, BLACK_PAWN, D3, WHITE_KNIGHT);
    if(val > 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    }

    testpos = fen_to_position("1k1r3q/1ppn3p/p4b2/4P3/8/P2N2P1/1PP1R1BP/2K1Q3 b - -");
    val = see(&testpos, E5, WHITE_PAWN, D7, BLACK_KNIGHT);
        if(val > 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    }

    testpos = fen_to_position("rnkr3b/6pp/2p1b3/p3p1N1/2q1N3/1R3Q2/P1PPPP1P/2BKR3 w - -");
    val = see(&testpos, E6, BLACK_BISHOP, G5, WHITE_KNIGHT);
    if(val < 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    } 

    printf("Static exchange tests passed!\n");
    #endif //SEE_TEST 

    #ifdef PYTHON
    python_close();
    #endif

    printf("\n--------------------------------------------------------------------------\n");

    return 0;
}
