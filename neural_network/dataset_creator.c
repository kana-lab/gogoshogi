#include "../Game.h"
#include "../Board.h"
#include "nn_shogi.c"
#include "random_move.c"

#include <stdio.h>
#include <string.h>

#define LEN_INITIAL_BOARDS 48245
#define INITIAL_BOARDS_FILE "initial_boards_48245.txt"
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


void save_self_match_dataset(Game *game, NNAI *player, char dataset[]) {
    // 自己対戦の結果からデータセットを作成する.
    // 強化学習におけるモンテカルロ法のQ値更新方法を参考にした.
    double alpha = 0.9;
    FILE *fp = fopen(dataset, "a");

    double q = 0.0;

    for (int i = game->history_len-1; 0 <= i; i--) {
        Hash h = reverse_hash(game->history[i]);
        fprintf(fp, "%llu %llu %lf\n", h.lower, h.upper, q);
        Board b = decode(h);
        q = (1.0-alpha)*nn_evaluate(&player->nn,&b) + alpha*q;
        q = 1.0 - q;
    }
    fclose(fp);
}


int create_dataset(PlayerInterface *first, PlayerInterface *second, char dataset_save_file[], int epoch, char dataset_mode[], Hash *initial_boards, int len_initial_boards) {
    // firstとsecondで対戦を行い, 学習データを生成する.

    // dataset_save_fileの中身を削除する.
    FILE *fp = fopen(dataset_save_file, "w");
    fclose(fp);

    int results_count[3] = {0, 0, 0};
    int sum_history_len = 0;
    bool *hash_table = calloc(HASH_MOD, sizeof(bool));

    for (int i = 0; i < epoch; i++) {
        // 初期化済みのゲームクラスを作る.
        Game game = create_game(MAX_TURN);
        if (initial_boards != NULL) {
            game.current = decode(initial_boards[rand()%len_initial_boards]);
            game.history[0] = reverse_hash(encode(&game.current));
        }

        // 対戦を行う.
        int winner = play(&game, first, second, false);

        // 盤面を保存する.
        if (strcmp(dataset_mode, "chackmate") == 0)
            save_checkmate_board(&game, hash_table, dataset_save_file);
        else if (strcmp(dataset_mode, "initial") == 0)
            save_initial_board(&game, hash_table, dataset_save_file);
        else if (strcmp(dataset_mode, "self_match") == 0)
            save_self_match_dataset(&game, (NNAI*) first, dataset_save_file);
        else
            debug_print("error; unknown dataset_mode");

        // 勝利回数などを記録する.
        results_count[winner+1]++;
        sum_history_len += game.history_len;

        // gameを削除する.
        destruct_game(&game);
    }

    debug_print("first win : %lf%%", (double) results_count[2] * 100.0 / epoch);
    debug_print("second win: %lf%%", (double) results_count[0] * 100.0 / epoch);
    debug_print("draw      : %lf%%", (double) results_count[1] * 100.0 / epoch);
    debug_print("average finish turn: %lf", (double) sum_history_len / epoch);

    return sum_history_len;
}


void self_match_learning(char model_file_name[], int epoch100) {
    // 自己対戦による強化学習を行う.
    // 100 * epoch100 対戦だけ行う.

    // 初期盤面を読み込む.
    Hash *initial_boards = malloc(LEN_INITIAL_BOARDS * sizeof(Hash));
    FILE *fp = fopen(INITIAL_BOARDS_FILE, "r");
    for (int i = 0; i < LEN_INITIAL_BOARDS; i++) {
        Hash h;
        fscanf(fp, "%llu %llu", &h.lower, &h.upper);
        initial_boards[i] = h;
    }
    fclose(fp);

    char *dataset = "self_match.txt";

    for (int i = 0; i < epoch100; i++) {
        debug_print("------------------------------");
        debug_print("epoch %d/%d", i+1, epoch100);

        // プレイヤーを初期化する.
        NNAI first = create_read1_ai(model_file_name);
        NNAI second = create_read1_ai(model_file_name);

        // データセットを作成する.
        int sum_history_len = create_dataset((PlayerInterface *) &first, (PlayerInterface *) &second, dataset, 100, "self_match", initial_boards, LEN_INITIAL_BOARDS);

        // 学習を行う.
        learn_dataset(dataset, sum_history_len, 0, model_file_name, model_file_name);
    }
}
