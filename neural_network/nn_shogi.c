#include "../Game.h"
#include "../Board.h"
#include "neural_network.c"

#define INPUT_SIZE 585
/*
#define TRAIN_SIZE 360000
#define TEST_SIZE 96554
#define DATASET "checkmates456554.txt"
#define WEIGHTS_FILE "weights585.txt"

#define LEN_ARRAY(a) ((sizeof(a)) / (sizeof(a[0])))
*/

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


void learn_dataset(NeuralNetwork *nn, char dataset[], int train_size, int test_size, char load_file[], char save_file[]){
    // datasetを学習する.
    // load_file == NULL のときは重みを読み込まない.
    // save_file == NULL のときは重みを保存しない.

    // 入出力サイズを取得する.
    int input_size = nn->affine[0].n;
    int output_size = nn->sigmoid.len;

    // データセットを準備する.
    // 学習データは左右を反転させて2倍にする.

    // メモリを確保する.
    double **X_train, **y_train, **X_test, **y_test;
    X_train = malloc(2 * train_size * sizeof(double *));
    y_train = malloc(2 * train_size * sizeof(double *));
    X_test = malloc(test_size * sizeof(double *));
    y_test = malloc(test_size * sizeof(double *));
    for (int i = 0; i < 2 * train_size; i++) {
        X_train[i] = malloc(input_size * sizeof(double));
        y_train[i] = malloc(output_size * sizeof(double));
    }
    for (int i = 0; i < test_size; i++) {
        X_test[i] = malloc(input_size * sizeof(double));
        y_test[i] = malloc(output_size * sizeof(double));
    }

    // 学習データを読み込む.
    FILE *fp = fopen(dataset, "r");
    for (int i = 0; i < train_size + test_size; i++) {
        Hash h;
        double ans;
        fscanf(fp, "%llu %llu %lf", &h.lower, &h.upper, &ans);
        Board b = decode(h);
        if (i < train_size) {
            board_to_vector(&b, X_train[i]);
            y_train[i][0] = ans;
            // Data Augmentation
            mirror_board(&b);
            board_to_vector(&b, X_train[i + train_size]);
            y_train[i + train_size][0] = ans;
        } else {
            board_to_vector(&b, X_test[i - train_size]);
            y_test[i - train_size][0] = ans;
        }
    }
    fclose(fp);

    // 重みを読み込む
    if (load_file != NULL)
        nn_load_weights(nn, load_file);

    // モデルを学習させる.
    double lr = 0.001;
    int epoch = 5;
    nn_fit(nn, X_train, y_train, 2 * train_size, X_test, y_test, test_size, lr, epoch);

    // 重みを保存する.
    if (save_file != NULL)
        nn_save_weights(nn, save_file);
    
    // メモリを解放する.
    for (int i = 0; i < 2 * train_size; i++) {
        free(X_train[i]);
        free(y_train[i]);
    }
    for (int i = 0; i < test_size; i++) {
        free(X_test[i]);
        free(y_test[i]);
    }
    free(X_train);
    free(y_train);
    free(X_test);
    free(y_test);    
}


double nn_evaluate(NeuralNetwork *nn, const Board *b){
    // 局面の評価値(0.0~1.0)を返す.
    // 評価値が高いほど, 手番側が優勢である.
    board_to_vector(b, nn->affine[0].x);
    nn_predict(nn, nn->affine[0].x, NULL, 0.0);
    return nn->sigmoid.out[0];
}

/*
int main(void){
    // learn_datasetの使用例

    // モデルを準備する.
    NeuralNetwork nn;
    int depth = 3;
    int sizes[4] = {INPUT_SIZE, 32, 32, 1};
    assert(LEN_ARRAY(sizes) - 1 == depth);
    nn_init(&nn, depth, sizes);

    // モデルを学習させる.
    learn_dataset(&nn, DATASET, TRAIN_SIZE, TEST_SIZE, NULL, WEIGHTS_FILE);

    // モデルを削除する.
    nn_free(&nn);

    return 0;
}
*/
