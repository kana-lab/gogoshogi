#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H


typedef struct {
    // 最適化関数で用いられる変数の定義
    double *wv;
    double *bv;
    double *ws;
    double *bs;
} Velocities;


typedef struct {
    int n;       // 入力ノード数
    int m;       // 出力ノード数
    double *w;   // weights(m x n)
    double *b;   // bias(m)
    double *x;   // 順伝播の入力, 逆伝播の出力dxと兼用
    double *out; // 順伝播の出力, 逆伝播の入力doutと兼用
    double *dw;  // wの微分
    double *db;  // bの微分
    double *dx;  // xの微分
} AffineLayer;


typedef struct {
    int len;     // 入出力ノード数
    double *out; // 出力結果
    double *dout;// 逆伝播の入力
} SigmoidLayer;


typedef struct {
    int len;     // 入出力ノード数
    double *out; // 出力
} ReluLayer;


typedef struct tagNeuralNetwork {
    // Neural Network
    // Affine[0] -> ReLU[0] -> ... -> ReLU[depth-2] -> Affine[depth-1] -> Sigmoid
    int depth;
    AffineLayer *affine;
    Velocities *velocities;
    ReluLayer *relu;
    SigmoidLayer sigmoid;
} NeuralNetwork;

void nn_init(NeuralNetwork *nn, int depth, int sizes[depth + 1]);

void nn_free(NeuralNetwork *nn);

void nn_load_model(NeuralNetwork *nn, char load_file[]);

int get_prioritized_actions(NeuralNetwork *nn, const Game *game, Action return_actions[LEN_ACTIONS]);


typedef struct tagNNAI {
    Action (*get_action)(struct tagNNAI *self, const Game *game);

    NeuralNetwork nn;
} NNAI;

NNAI create_minimax_ai(char load_file_name[]);

NNAI create_read1_ai(char load_file_name[]);


#endif  /* NEURAL_NETWORK_H */
