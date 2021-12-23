#include "../Game.h"
#include "../Board.h"

#include <stdlib.h>


/*
動かす駒がランダムなAIを実装した.
*/


int action_to_int(const Action *action) {
    // actionで動かす駒の元の位置を, 0以上30以下の整数にして返す.
    if (action->from_stock)
        return 25 + action->from_stock;
    else
        return 5 * action->from_x + action->from_y;
}


Action choose_random_action(const Action actions[], int len_actions) {
    // 動かす駒がランダムになるように指手を選ぶ.

    // 動かす駒を選ぶ.
    int candidates[30], len_candidates = 0;
    for (int i = 0; i < len_actions; i++) {
        int exist = 0;
        int action_number = action_to_int(&actions[i]);
        for (int j = 0; j < len_candidates; j++) {
            if (candidates[j] == action_number) {
                // 既に列挙していた場合はスキップする.
                exist = 1;
                break;
            }
        }
        if (exist == 0)
            // 新しい駒の場合
            candidates[len_candidates++] = action_number;
    }
    int move_piece = candidates[rand() % len_candidates];

    // 指手を選ぶ.
    len_candidates = 0;
    for (int i = 0; i < len_actions; i++) {
        if (move_piece == action_to_int(&actions[i]))
            candidates[len_candidates++] = i;
    }

    // 指手を返す.
    return actions[candidates[rand() % len_candidates]];
}


Action random_move_ai(const Game *game) {
    // 動かす駒がランダムなAI
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_useful_actions_with_tfr(game, all_actions);
    return choose_random_action(all_actions, len_all_actions);
}


// random_move_aiのPlayerInterfaceクラスのものを作る.


// PlayerInterfaceクラスを継承
typedef struct tagAI {
    Action (*get_action)(struct tagAI *self, const Game *game);
} AI;


Action get_random_move_ai_action(AI *self, const Game *game) {
    return random_move_ai(game);
}


AI create_random_move_ai() {
    return (AI) {
            .get_action=get_random_move_ai_action,
    };
}
