#include "../Game.h"
#include "../Board.h"
#include "random_move.c"

#include <stdio.h>

#define HASH_MOD 9999991 // 10^7以下の最大素数


/*
教師データを生成するための関数を実装した.
*/


int get_hash_mod(const Hash *h) {
    return (h->lower ^ h->upper) % HASH_MOD;
}


void save_checkmate_board(Game *game, bool hash_table[HASH_MOD], char dataset[]) {
    // hash_tableとhashが衝突しないときに, gameをdatasetに保存する.
    FILE *fp = fopen(dataset, "a");
    Hash win_hash = reverse_hash(game->history[game->history_len - 2]);
    Hash lose_hash = reverse_hash(game->history[game->history_len - 1]);
    Board lose_board = decode(lose_hash);
    if (is_checkmate(&lose_board)) {
        if (!hash_table[get_hash_mod(&win_hash)]) {
            if (!hash_table[get_hash_mod(&lose_hash)]) {
                fprintf(fp, "%llu %llu 1.0\n", win_hash.lower, win_hash.upper);
                fprintf(fp, "%llu %llu 0.0\n", lose_hash.lower, lose_hash.upper);
                hash_table[get_hash_mod(&win_hash)] = true;
                hash_table[get_hash_mod(&lose_hash)] = true;
            }
        }
    }
    fclose(fp);
}


void save_initial_board(Game *game, bool hash_table[HASH_MOD], char dataset[]) {
    // hash_tableとhashが衝突しないときに, gameをdatasetに保存する.
    // 最初の10手に制限した局面を保存する.
    int save_max_turn = 10;
    FILE *fp = fopen(dataset, "a");
    for (int i = 0; i < save_max_turn; i++) {
        if (i == game->history_len)
            // iがgame->historyのindexを超えないようにする.
            break;
        Hash h = reverse_hash(game->history[i]);
        if (!hash_table[get_hash_mod(&h)]) {
            fprintf(fp, "%llu %llu\n", h.lower, h.upper);
            hash_table[get_hash_mod(&h)] = true;
        }
    }
    fclose(fp);
}


void create_dataset(PlayerInterface *first, PlayerInterface *second, char dataset_save_file[], int epoch) {
    // firstとsecondで対戦を行い, 学習データを生成する.

    int results_count[3] = {0, 0, 0};
    bool *hash_table = calloc(HASH_MOD, sizeof(bool));

    for (int i = 0; i < epoch; i++) {
        // 初期化済みのゲームクラスを作る.
        Game game = create_game(MAX_TURN);

        // 対戦を行う.
        int winner = play(&game, first, second);

        // 盤面を保存する.
        //save_checkmate_board(&game, hash_table, dataset_save_file);
        save_initial_board(&game, hash_table, dataset_save_file);

        // 勝利回数を記録する.
        results_count[winner+1]++;

        // gameを削除する.
        destruct_game(&game);
    }

    debug_print("first win : %lf%%", (double) results_count[2] * 100.0 / epoch);
    debug_print("second win: %lf%%", (double) results_count[0] * 100.0 / epoch);
    debug_print("draw      : %lf%%", (double) results_count[1] * 100.0 / epoch);
}
