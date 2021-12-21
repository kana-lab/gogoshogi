#include "../Game.h"
#include "../Board.h"

#include <stdio.h>


#define HASH_MOD 9999991 // 10^7以下の最大素数
#define DATASET "checkmates.txt"


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
    int len_all_actions = get_all_actions_with_tfr(game, all_actions);
    return choose_random_action(all_actions, len_all_actions);
}


int get_hash_mod(const Hash *h) {
    return (h->lower ^ h->upper) % HASH_MOD;
}


void save_board(Game *game, bool hash_table[HASH_MOD]) {
    // gameが, hash_tableとhashが衝突しないときにgameをdatasetに保存する.
    FILE *fp = fopen(DATASET, "a");
    Board win_board = game->history[game->history_len - 2];
    Board lose_board = game->history[game->history_len - 1];
    reverse_board(&win_board);
    reverse_board(&lose_board);
    //print_board_for_debug(&win_board);
    //print_board_for_debug(&lose_board);
    Hash win_hash = encode(&win_board);
    Hash lose_hash = encode(&lose_board);
    if (is_checkmate(&lose_board)) {
        if (hash_table[get_hash_mod(&win_hash)]) {
            if (hash_table[get_hash_mod(&lose_hash)]) {
                fprintf(fp, "%llu %llu 1.0\n", win_hash.lower, win_hash.upper);
                fprintf(fp, "%llu %llu 0.0\n", lose_hash.lower, lose_hash.upper);
                hash_table[get_hash_mod(&win_hash)] = false;
                hash_table[get_hash_mod(&lose_hash)] = false;
            }
        }
    }
    fclose(fp);
}
