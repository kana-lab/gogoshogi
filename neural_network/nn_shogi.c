#include "../Game.h"
#include "../Board.h"
#include "neural_network.c"

#define INPUT_SIZE 361


/*
ニューラルネットワークの学習を行う.
*/


void board_to_vector(const Board *b, bool is_first, double vec[INPUT_SIZE]) {
    // 盤面を361次元のベクトルに変換する.
    assert(INPUT_SIZE == 361);

    // vecを初期化する.
    for (int i = 0; i < INPUT_SIZE; i++)
        vec[i] = 0.0;
    
    // 盤面を1次元のベクトルに変換する.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (b->board[i][j] > 0)
                vec[5*i + j] = 1.0;
            else if (b->board[i][j] > 0)
                vec[25 + 5*i + j] = 1.0;
        }
    }

    // 持ち駒の情報を入力する.
    for (int i = 0; i < 5; i++) {
        vec[50 + i] = b->next_stock[i+1];
        vec[55 + i] = b->previous_stock[i+1];
    }

    // 駒のききを入力する.
    piece_moves_to_vector(b, vec, 60);
    Board b_copy = *b;
    reverse_board(&b_copy);
    piece_moves_to_vector(&b_copy, vec, 210);

    // 先手番か後手番かを入力する.
    assert(is_first == 0 || is_first == 1);
    vec[360] = (double)is_first;
}


void mirror_board(Board *b) {
    // 盤面を左右反転させる.
    // Data Augmentation に使える.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 2; j++)
            SWAP(b->board[i][j], b->board[i][4 - j]);
    }
}


void learn_dataset(char dataset[], int train_size, int test_size, char load_file[], char save_file[]){
    // datasetを学習する.
    // load_file == NULL のときは重みを読み込まない.
    // save_file == NULL のときは重みを保存しない.

    // NeuralNetworkを初期化する.
    NeuralNetwork nn;
    if (load_file == NULL) {
        int depth = 3;
        int sizes[4] = {INPUT_SIZE, 32, 32, 1};
        nn_init(&nn, depth, sizes);
    } else
        nn_load_model(&nn, load_file);

    // 入出力サイズを取得する.
    int input_size = nn.affine[0].n;
    int output_size = nn.sigmoid.len;

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
            board_to_vector(&b, rand()%2, X_train[i]);
            y_train[i][0] = ans;
            // Data Augmentation
            mirror_board(&b);
            board_to_vector(&b, rand()%2, X_train[i + train_size]);
            y_train[i + train_size][0] = ans;
        } else {
            board_to_vector(&b, rand()%2, X_test[i - train_size]);
            y_test[i - train_size][0] = ans;
        }
    }
    fclose(fp);

    // モデルを学習させる.
    double lr = 0.001;
    int epoch = 5;
    nn_fit(&nn, X_train, y_train, 2 * train_size, X_test, y_test, test_size, lr, epoch);

    // 重みを保存する.
    if (save_file != NULL)
        nn_save_model(&nn, save_file);
    
    // メモリを解放する.
    nn_free(&nn);

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


double nn_evaluate(NeuralNetwork *nn, bool is_first, const Board *b){
    // 局面の評価値(0.0~1.0)を返す.
    // 評価値が高いほど, 手番側が優勢である.
    board_to_vector(b, is_first, nn->affine[0].x);
    nn_forward(nn, nn->affine[0].x);
    return nn->sigmoid.out[0];
}


// NeuralNetworkを用いるPlayerInterfaceクラスのものを作る.


// PlayerInterfaceクラスを継承
typedef struct tagNNAI {
    Action (*get_action)(struct tagNNAI *self, const Game *game);
    NeuralNetwork nn;
} NNAI;


Action get_read1_ai_action(NNAI *self, const Game *game) {

    // 1手先の局面の(相手にとっての)評価値が最も低くなるような指手を返す.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_useful_actions_with_tfr(game, all_actions);

    int best_action = 0;
    double min_evaluation = 1.0;
    for (int i = 0; i < len_all_actions; i++) {
        do_action((Game *) game, all_actions[i]);
        double evaluation = nn_evaluate(&self->nn, 1-game->turn%2, &game->current);
        undo_action((Game *) game);
        if (evaluation < min_evaluation) {
            best_action = i;
            min_evaluation = evaluation;
        }
        /*
        // 各指手の評価値を出力したいときはコメントを外す.
        char buffer[32];
        Action action = all_actions[i];
        reverse_action(&action);
        action_to_string(action, buffer);
        printf("%s %lf, ", buffer, evaluation);
        */
    }

    return all_actions[best_action];
}


NNAI create_read1_ai(char load_file_name[]) {
    NNAI ai;
    ai.get_action = get_read1_ai_action;
    nn_load_model(&ai.nn, load_file_name);
    return ai;
}
