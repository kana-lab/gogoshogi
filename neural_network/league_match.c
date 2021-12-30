#include "../Game.h"
#include "../Board.h"
#include "beam_search.c"

#define PLAYER 6


/*
AI同士を対戦させ, 結果を出力する.
*/


int main(void) {

    // Playerの登録を行う.

    NNAI players[PLAYER];
    char *names[PLAYER];

    players[0] = create_read1_ai("nn_32x2_0.txt");
    names[0] = "32x2 almost random AI";

    players[1] = create_read1_ai("nn_32x2_1.txt");
    names[1] = "32x2 normal AI";

    players[2] = create_read1_ai("nn_64x2_0.txt");
    names[2] = "64x2 almost random AI";

    players[3] = create_read1_ai("nn_64x2_1.txt");
    names[3] = "64x2 normal AI";

    players[4] = create_beam_search_ai("nn_32x2_1.txt");
    names[4] = "32x2 beam search AI";

    players[5] = create_beam_search_ai("nn_64x2_1.txt");
    names[5] = "64x2 beam search AI";

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

    return 0;
}
