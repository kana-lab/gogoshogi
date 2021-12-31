#include "../Game.h"
#include "../Board.h"
#include "minimax.c"

#define PLAYER 4


/*
AI同士を対戦させ, 結果を出力する.

このバージョンの出力

Number 1: 32x2 minimax AI
Number 2: 64x2 minimax AI
Number 3: 128x2_64x2_32x2 minimax AI
Number 4: 128x2_64x2_32x2 minimax AI 2

League Match Results

○×○×
○○○×
○○○×
××○○

(i行目には Number i が先手のときの結果が表示されています.)
*/


void league_match(void) {

    // Playerの登録を行う.

    NNAI players[PLAYER];
    char *names[PLAYER];

    players[0] = create_minimax_ai("nn_32x2_1.txt");
    names[0] = "32x2 minimax AI";
    
    players[1] = create_minimax_ai("nn_64x2_1.txt");
    names[1] = "64x2 minimax AI";

    players[2] = create_minimax_ai("nn_128x2_64x2_32x2_1.txt");
    names[2] = "128x2_64x2_32x2 minimax AI";

    players[3] = create_minimax_ai("nn_128x2_64x2_32x2_2.txt");
    names[3] = "128x2_64x2_32x2 minimax AI 2";

    // Player同士を対戦させる.

    char *results[PLAYER][PLAYER];

    for (int i = 0; i < PLAYER; i++) {
        printf("Number %d: %s\n", i+1, names[i]);

        // players[i]と他のplayerを対戦させる.
        for (int j = 0; j < PLAYER; j++) {
            // 対戦する.
            Game game = create_game(MAX_TURN);
            int result = play(&game, (PlayerInterface *) &players[i], (PlayerInterface *) &players[j], false);
            destruct_game(&game);

            // 結果を保存する.
            if (result == 1)
                results[i][j] = "○";
            else if (result == 0)
                results[i][j] = "△";
            else
                results[i][j] = "×";
        }
    }

    // 結果を出力する.

    printf("\nLeague Match Results\n\n");
    for (int i = 0; i < PLAYER; i++) {
        for (int j = 0; j < PLAYER; j++)
            printf("%s", results[i][j]);
        printf("\n");
    }
    printf("\n(i行目には Number i が先手のときの結果が表示されています.)\n");

    // メモリを解放する.

    for (int i = 0; i < PLAYER; i++) {
        nn_free(&players[i].nn);
    }
}
