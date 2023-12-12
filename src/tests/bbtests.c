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
#include <inttypes.h>
#include <time.h>
#include "../bitboard/bitboard.h"
#include "../bitboard/magic.h"
#include "../movement.h"
#include "../util.h"

#define MOVE_GEN_TEST
#define MOVE_MAKE_TEST

int testBB(void) {
    generateMasks();
    generateMagics();

    


    #ifdef MOVE_GEN_TEST
    FILE *file;
    char line[1024];
    Position pos;
    Move moveList[MAX_MOVES];
    int size, expectedMoves;
    
    printf("\n---------------------------------- MOVE GEN TESTING ----------------------------------\n\n");
    

    file = fopen("perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        char *fenEnd = strchr(line, ';');
        if (fenEnd) *fenEnd = '\0';
        char *fen = line;
        pos = fenToPosition(fen);
        if (fenEnd) *fenEnd = ';'; 

        char *token;
        while ((token = strtok(fenEnd ? fenEnd + 1 : NULL, " ;")) != NULL) {
            if (token[0] == 'D' && token[1] == '1') {
                expectedMoves = atoi(token + 3);
                size = generateLegalMoves(pos, moveList);
                if (size != expectedMoves) {
                    printf("Failed to get correct amount of moves for Position %s, correct: %d, found: %d\n", fen, expectedMoves, size);
                    printPosition(pos);
                    for (int i = 0; i < size; i++) {
                        printMove(moveList[i]);
                    }
                    return -1;
                }
                break;
            }
        }
    }

    fclose(file);

    printf("Finished Depth 1 Position Check \n");
    #endif

    #ifdef MOVE_GEN_TEST
    printf("\n---------------------------------- MOVE MAKING TESTING ----------------------------------\n\n");
    time_t t;
    srand((unsigned) time(&t));

    
    printf("Quick Crash Check\n");
    for(int j = 0; j < 10000; j++){
        char* FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        pos = fenToPosition(FEN);
        size = generateLegalMoves(pos, moveList);
        int i = 0;
        while(size != 0 && i < 1000){
            int randMove = rand() % size;
            makeMove(&pos, moveList[randMove]);
            size = generateLegalMoves(pos, moveList);
            i++;
        }
    }
    printf("Crash Check Complete.\n");

    /*
    char* FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; //Default
    //FEN = "8/7p/5K1k/8/6Pp/7P/5P2/8 b - g3 0 1"; //EP
    //FEN = "8/PPPPPPPP/8/8/2K2k2/8/pppppppp/8 b - - 0 1"; //Promotion Madness
    FEN = "4k3/2r1nr2/p7/P7/5pbP/8/4P3/3K4 w - - 0 1"; //En passant stuff
    pos = fenToPosition(FEN);
    printf("Starting Posistion:\n");
    printPosition(pos);

    printf("Press ENTER key to Continue\n");  
    getchar();  
    size = generateLegalMoves(pos, moveList);
    printf("Legal moves:\n");
    for (int i = 0; i < size; i++) {
        printMove(moveList[i]);
    }
    int i = 0;
    while(size != 0 && i < 10000){
        printf("Making move:\n");
        int randMove = rand() % size;
        printMove(moveList[randMove]);
        makeMove(&pos, moveList[randMove]);
        printPosition(pos);
        size = generateLegalMoves(pos, moveList);
        i++;
        printf("Press ENTER key to Continue\n");  
        getchar();   
    }  
    char* FEN = "5k2/1R6/8/1N1rPpK1/n7/8/2P5/8 w - f6 0 1";
    pos = fenToPosition(FEN);
    printf("Starting Posistion:\n");
    printPosition(pos);
    size = generateLegalMoves(pos, moveList);
    printf("Legal moves:\n");
    for (int i = 0; i < size; i++) {
        printMove(moveList[i]);
    }
    */
    

    #endif





    #ifdef PERF_TEST
    printf("\n----------------------------------\n");

    FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos = fenToPosition(FEN);
    for(int depth = 1; depth < 5; depth++){
        uint64_t num_moves = perft(depth, pos);
        printf("Perft output is %lld for depth %d\n", (long long unsigned)num_moves, depth);
    }
    #endif
    


    

    
    
    
    printf("\n--------------------------------------------------------------------------------------\n");
    return 0;
    
}
