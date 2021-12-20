#include "Game.h"
#include "neural_network/neural_network.c"
#include "hash.c"

#define INPUT_SIZE 585
#define TRAIN_SIZE 360000
#define TEST_SIZE 96554
#define DATASET "checkmates456554.txt"
#define WEIGHTS_FILE "weights585.txt"


/*
ニューラルネットワークの学習を行う.
*/


static int move_matrix_x[MAX_PIECE_NUMBER + 1][8] = {
        {},                           // EMPTY
        {-1},                         // 歩
        {-1, -1, 1,  1},              // 角1
        {-1, 1,  0,  0},              // 飛1
        {-1, -1, -1, 1, 1},           // 銀
        {-1, -1, -1, 0, 0, 1},        // 金
        {-1, -1, -1, 0, 0, 1, 1, 1},  // 王
        {-1, -1, -1, 0, 0, 1},        // と
        {-1, 1,  0,  0},              // 馬 - 角
        {-1, -1, 1,  1},              // 龍 - 飛
        {-1, -1, -1, 0, 0, 1}         // 全
};

static int move_matrix_y[MAX_PIECE_NUMBER + 1][8] = {
        {},                            // EMPTY
        {0},                           // 歩
        {-1, 1, -1, 1},                // 角1
        {0,  0, -1, 1},                // 飛1
        {-1, 0, 1,  -1, 1},            // 銀
        {-1, 0, 1,  -1, 1, 0},         // 金
        {-1, 0, 1,  -1, 1, -1, 0, 1},  // 王
        {-1, 0, 1,  -1, 1, 0},         // と
        {0,  0, -1, 1},                // 馬 - 角
        {-1, 1, -1, 1},                // 龍 - 飛
        {-1, 0, 1,  -1, 1, 0},         // 全
};

static int move_length[MAX_PIECE_NUMBER + 1] = {0, 1, 4, 4, 5, 6, 8, 6, 4, 4, 6};

void count_connections(const Board *b, double counts[5][5]) {
    /*
    各マスについて, ききのある駒の数を数える.
    */

    // 初期化する.
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++)
            counts[i][j] = 0.0;
    }
    
    // 数える.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = b->board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            if (piece != KAKU && piece != HISHA) {
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する.
                for (int k = 0; k < move_length[piece]; k++) {
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    if (0 <= x && x < 5 && 0 <= y && y < 5){// && b->board[x][y] <= EMPTY) {
                        counts[x][y] += 1.0;
                    }
                }
            }
            if (piece % NARI == KAKU || piece % NARI == HISHA) {
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する.
                for (int k = 0; k < move_length[piece % NARI]; k++) {
                    int x = i + move_matrix_x[piece % NARI][k];
                    int y = j + move_matrix_y[piece % NARI][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5){// && b->board[x][y] <= EMPTY) {
                        counts[x][y] += 1.0;
                        /*
                        if (b->board[x][y] < EMPTY)
                            // 相手の駒を取ったとき
                            break;
                        */
                        x += move_matrix_x[piece % NARI][k];
                        y += move_matrix_y[piece % NARI][k];
                    }
                }
            }
        }
    }
}


void board_to_vector(const Board *b, double vec[INPUT_SIZE]){
    // 盤面を1次元のベクトルに変換する.

    // vecを初期化する.
    for (int i = 0; i < INPUT_SIZE; i++)
        vec[i] = 0.0;

    // 盤上の情報を入力する.
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++)
            vec[21*(5*i+j) + b->board[i][j]+MAX_PIECE_NUMBER] = 1.0;
    }

    // 持ち駒の情報を入力する.
    for (int i = 0; i < 5; i++){
        vec[21*25 + i] = (double)b->next_stock[i+1];
        vec[21*25+5 + i] = (double)b->previous_stock[i+1];
    }

    // 自分の駒のききの数を入力する.
    double counts[5][5];
    count_connections(b, counts);
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++)
            vec[21*25+10 + 5*i + j] = counts[i][j];
    }

    // 相手の駒のききの数を入力する.
    // 座標が180度ずれているが, 気にしないことにする.
    Board b_copy = *b;
    reverse_board(&b_copy);
    count_connections(&b_copy, counts);
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++)
            vec[21*25+10+25 + 5*i + j] = counts[i][j];
    }
}


void mirror_board(Board *b){
    // 盤面を左右反転させる.
    // Data Augmentation に使える.
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 2; j++)
            SWAP(b->board[i][j], b->board[i][4-j]);
    }
}


int main(void){

    // モデルの準備
    NeuralNetwork *nn;
    nn = malloc(sizeof(NeuralNetwork));
    int model_size[4] = {INPUT_SIZE, 32, 32, 1};
    nn_init(nn, model_size);

    // データセットの準備
    FILE *fp = fopen(DATASET, "r");
    double **X_train, **y_train, **X_test, **y_test;
    X_train = malloc(2*TRAIN_SIZE * sizeof(double*));
    y_train = malloc(2*TRAIN_SIZE * sizeof(double*));
    X_test = malloc(TEST_SIZE * sizeof(double*));
    y_test = malloc(TEST_SIZE * sizeof(double*));
    for (int i = 0; i < 2*TRAIN_SIZE; i++){
        X_train[i] = malloc(INPUT_SIZE * sizeof(double));
        y_train[i] = malloc(1 * sizeof(double));
    }
    for (int i = 0; i < TEST_SIZE; i++){
        X_test[i] = malloc(INPUT_SIZE * sizeof(double));
        y_test[i] = malloc(1 * sizeof(double));
    }
    for (int i = 0; i < TRAIN_SIZE+TEST_SIZE; i++){
        Hash h;
        double ans;
        fscanf(fp, "%llu %llu %lf", &h.lower, &h.upper, &ans);
        Board b = decode(h);
        if (i < TRAIN_SIZE){
            board_to_vector(&b, X_train[i]);
            y_train[i][0] = ans;
            // Data Augmentation
            mirror_board(&b);
            board_to_vector(&b, X_train[i+TRAIN_SIZE]);
            y_train[i+TRAIN_SIZE][0] = ans;
        }else{
            board_to_vector(&b, X_test[i-TRAIN_SIZE]);
            y_test[i-TRAIN_SIZE][0] = ans;
        }
    }
    fclose(fp);

    // 重みの読み込み
    //nn_load_weights(nn, WEIGHTS_FILE);

    // モデルの学習
    double lr = 0.001;
    int epoch = 5;
    nn_fit(nn, X_train, y_train, 2*TRAIN_SIZE, X_test, y_test, TEST_SIZE, lr, epoch);

    // 重みの保存
    //nn_save_weights(nn, WEIGHTS_FILE);

    return 0;
}
