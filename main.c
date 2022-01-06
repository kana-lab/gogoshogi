#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Game.h"
#include "MultiThread.h"
#include "neural_network/neural_network.h"


/*******************************
 * ユーザーを表すクラスUserの定義
 *******************************/

// PlayerInterfaceクラスを継承
typedef struct tagUser {
    Action (*get_action)(struct tagUser *self, const Game *game);

    char buf[32];  // 入出力時に利用するバッファ
} User;


Action get_user_action(User *self, const Game *game) {
    // 前のプレイヤーの行動を表示し、ユーザーからの入力を待つ

    // 前のプレイヤーの行動を表示
    Action previous_action = get_previous_action(game);
    Action null_action = {};
    if (!action_equal(&previous_action, &null_action)) {
        if (game->turn % 2)  // 自分が先手であるとき
            reverse_action(&previous_action);
        action_to_string(previous_action, self->buf);
        puts(self->buf);
    }

    // ユーザーからの入力を受ける
    scanf("%31s", self->buf);

    Action action;
    if (!string_to_action(self->buf, &action)) {
        debug_print("in get_user_action: invalid input string.");

        // abort_game()関数を消したので若干汚い…あった方が良いのか否か
        puts("You Lose");
        exit(1);
    }

    if (game->turn % 2 == 0)  // 自分が後手のとき
        reverse_action(&action);

    return action;
}


User create_user() {
    return (User) {
            .get_action=get_user_action,
            .buf={}
    };
}


/******************************
 * 以下main関数
 ******************************/

int main(int argc, char *argv[]) {
    // 引数の個数をチェック
    if (argc != 2) {
        puts("the number of command line arguments must be 2.");
        return -1;
    }

    // 先手か後手か
    bool is_user_first;
    if (!strcmp(argv[1], "0")) {
        is_user_first = true;
    } else if (!strcmp(argv[1], "1")) {
        is_user_first = false;
    } else {
        puts("invalid command line argument.");
        return -1;
    }

    // 初期化済みのゲームクラスを作る
    Game game = create_game(MAX_TURN);

    // プレイヤーの宣言
    char *path = "neural_network/nn_128x2_64x2_32x2_1.txt";
    MultiExplorer ai = create_multi_explorer(&game, is_user_first, path);
    User user = create_user();

    // ゲームを行い、勝者を決める
    int winner;
    if (is_user_first) {
        winner = play(&game, (PlayerInterface *) &user, (PlayerInterface *) &ai, true);
    } else {
        winner = play(&game, (PlayerInterface *) &ai, (PlayerInterface *) &user, true);
    }

    // 勝敗の表示
    if (winner == 1) {
        puts((is_user_first) ? "You Win" : "You Lose");
    } else if (winner == -1) {
        puts((is_user_first) ? "You Lose" : "You Win");
    } else {
        puts("Draw");
    }

    // 各変数の解放
    destruct_game(&game);
    destruct_multi_explorer(&ai);

    return 0;
}
