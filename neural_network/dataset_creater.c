#include "../Game.h"
#include "../Board.h"

#include <stdio.h>

#define HASH_MOD 9999991 // 10^7以下の最大素数
#define DATASET "checkmates.txt"


/*
教師データを生成するための関数を実装した.
*/


int get_hash_mod(const Hash *h) {
    return (h->lower ^ h->upper) % HASH_MOD;
}


void save_board(Game *game, bool hash_table[HASH_MOD]) {
    // gameが, hash_tableとhashが衝突しないときにgameをdatasetに保存する.
    FILE *fp = fopen(DATASET, "a");
    Hash win_hash = reverse_hash(game->history[game->history_len - 2]);
    Hash lose_hash = reverse_hash(game->history[game->history_len - 1]);
    Board lose_board = decode(lose_hash);
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
