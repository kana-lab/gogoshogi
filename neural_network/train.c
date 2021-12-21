#include "../Game.h"
#include "../Board.h"
#include "neural_network.c"

#define INPUT_SIZE 585
#define TRAIN_SIZE 360000
#define TEST_SIZE 96554
#define DATASET "checkmates456554.txt"
#define WEIGHTS_FILE "weights585.txt"


/*
ニューラルネットワークの学習を行う.
*/


void board_to_vector(const Board *b, double vec[INPUT_SIZE]) {
    // 盤面を1次元のベクトルに変換する.

    // vecを初期化する.
    for (int i = 0; i < INPUT_SIZE; i++)
        vec[i] = 0.0;

    // 盤上の情報を入力する.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            vec[21 * (5 * i + j) + b->board[i][j] + MAX_PIECE_NUMBER] = 1.0;
    }

    // 持ち駒の情報を入力する.
    for (int i = 0; i < 5; i++) {
        vec[21 * 25 + i] = (double) b->next_stock[i + 1];
        vec[21 * 25 + 5 + i] = (double) b->previous_stock[i + 1];
    }

    // 自分の駒のききの数を入力する.
    double counts[5][5];
    count_connections(b, counts);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            vec[21 * 25 + 10 + 5 * i + j] = counts[i][j];
    }

    // 相手の駒のききの数を入力する.
    // 座標が180度ずれているが, 気にしないことにする.
    Board b_copy = *b;
    reverse_board(&b_copy);
    count_connections(&b_copy, counts);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            vec[21 * 25 + 10 + 25 + 5 * i + j] = counts[i][j];
    }
}


void mirror_board(Board *b) {
    // 盤面を左右反転させる.
    // Data Augmentation に使える.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 2; j++)
            SWAP(b->board[i][j], b->board[i][4 - j]);
    }
}


int main(void) {

    // モデルの準備
    NeuralNetwork nn;
    int depth = 3;
    int sizes[4] = {INPUT_SIZE, 32, 32, 1};
    nn_init(&nn, depth, sizes);

    // データセットの準備
    FILE *fp = fopen(DATASET, "r");
    double **X_train, **y_train, **X_test, **y_test;
    X_train = malloc(2 * TRAIN_SIZE * sizeof(double *));
    y_train = malloc(2 * TRAIN_SIZE * sizeof(double *));
    X_test = malloc(TEST_SIZE * sizeof(double *));
    y_test = malloc(TEST_SIZE * sizeof(double *));
    for (int i = 0; i < 2 * TRAIN_SIZE; i++) {
        X_train[i] = malloc(INPUT_SIZE * sizeof(double));
        y_train[i] = malloc(1 * sizeof(double));
    }
    for (int i = 0; i < TEST_SIZE; i++) {
        X_test[i] = malloc(INPUT_SIZE * sizeof(double));
        y_test[i] = malloc(1 * sizeof(double));
    }
    for (int i = 0; i < TRAIN_SIZE + TEST_SIZE; i++) {
        Hash h;
        double ans;
        fscanf(fp, "%llu %llu %lf", &h.lower, &h.upper, &ans);
        Board b = decode(h);
        if (i < TRAIN_SIZE) {
            board_to_vector(&b, X_train[i]);
            y_train[i][0] = ans;
            // Data Augmentation
            mirror_board(&b);
            board_to_vector(&b, X_train[i + TRAIN_SIZE]);
            y_train[i + TRAIN_SIZE][0] = ans;
        } else {
            board_to_vector(&b, X_test[i - TRAIN_SIZE]);
            y_test[i - TRAIN_SIZE][0] = ans;
        }
    }
    fclose(fp);

    // 重みの読み込み
    //nn_load_weights(&nn, WEIGHTS_FILE);

    // モデルの学習
    double lr = 0.001;
    int epoch = 5;
    nn_fit(&nn, X_train, y_train, 2 * TRAIN_SIZE, X_test, y_test, TEST_SIZE, lr, epoch);

    // 重みの保存
    //nn_save_weights(&nn, WEIGHTS_FILE);

    return 0;
}
