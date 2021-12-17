#include "layers.c"


/*
ニューラルネットワークを実装した.
*/


typedef struct{
    // Neural Network
    // Affine -> ReLU -> Affine -> ReLU -> Affine -> Sigmoid
    AffineLayer *affine1;
    Velocities *v1;
    ReluLayer *relu1;
    AffineLayer *affine2;
    Velocities *v2;
    ReluLayer *relu2;
    AffineLayer *affine3;
    Velocities *v3;
    SigmoidLayer *sigmoid1;
}NeuralNetwork;

void nn_init(NeuralNetwork *nn, int sizes[4]){
    // NeuralNetworkを初期化する.
    nn->affine1 = malloc(sizeof(AffineLayer));
    nn->v1 = malloc(sizeof(Velocities));
    nn->relu1 = malloc(sizeof(ReluLayer));
    nn->affine2 = malloc(sizeof(AffineLayer));
    nn->v2 = malloc(sizeof(Velocities));
    nn->relu2 = malloc(sizeof(ReluLayer));
    nn->affine3 = malloc(sizeof(AffineLayer));
    nn->v3 = malloc(sizeof(Velocities));
    nn->sigmoid1 = malloc(sizeof(SigmoidLayer));
    affine_init_with_he(nn->affine1, sizes[0], sizes[1]);
    velocities_init(nn->v1, nn->affine1);
    relu_init(nn->relu1, sizes[1]);
    affine_init_with_he(nn->affine2, sizes[1], sizes[2]);
    velocities_init(nn->v2, nn->affine2);
    relu_init(nn->relu2, sizes[2]);
    affine_init_with_xavier(nn->affine3, sizes[2], sizes[3]);
    velocities_init(nn->v3, nn->affine3);
    sigmoid_init(nn->sigmoid1, sizes[3]);
}

void nn_forward(NeuralNetwork *nn, const double x[], double y[]){
    // NeuralNetworkに入力xを与え, 出力をyに代入する.
    // affine1
    double *x1;
    x1 = malloc(nn->affine1->m * sizeof(double));
    affine_forward(nn->affine1, x, x1);
    // relu1
    double *x2;
    x2 = malloc(nn->relu1->len * sizeof(double));
    relu_forward(nn->relu1, x1, x2);
    // affine2
    double *x3;
    x3 = malloc(nn->affine2->m * sizeof(double));
    affine_forward(nn->affine2, x2, x3);
    // relu2
    double *x4;
    x4 = malloc(nn->relu2->len * sizeof(double));
    relu_forward(nn->relu2, x3, x4);
    // affine3
    double *x5;
    x5 = malloc(nn->affine3->m * sizeof(double));
    affine_forward(nn->affine3, x4, x5);
    // sigmoid1
    sigmoid_forward(nn->sigmoid1, x5, y);
    // メモリを解放する.
    free(x1);
    free(x2);
    free(x3);
    free(x4);
    free(x5);
}

void nn_backward(NeuralNetwork *nn, const double y[]){
    // 誤差を逆伝播させる.
    // sigmoid1
    double *y1;
    y1 = malloc(nn->sigmoid1->len * (sizeof(double)));
    sigmoid_backward(nn->sigmoid1, y, y1);
    // affine3
    double *y2;
    y2 = malloc(nn->affine3->n * (sizeof(double)));
    affine_backward(nn->affine3, y1, y2);
    // relu2
    double *y3;
    y3 = malloc(nn->relu2->len * (sizeof(double)));
    relu_backward(nn->relu2, y2, y3);
    // affine2
    double *y4;
    y4 = malloc(nn->affine2->n * (sizeof(double)));
    affine_backward(nn->affine2, y3, y4);
    // relu1
    double *y5;
    y5 = malloc(nn->relu1->len * (sizeof(double)));
    relu_backward(nn->relu1, y4, y5);
    // affine1
    double *y6;
    y6 = malloc(nn->affine1->n * (sizeof(double)));
    affine_backward(nn->affine1, y5, y6);
    // メモリを解放する.
    free(y1);
    free(y2);
    free(y3);
    free(y4);
    free(y5);
    free(y6);
}

double nn_predict(NeuralNetwork *nn, const double x[], const double t[], double y[], double lr, int learn){
    /*
    NeuralNetworkに入力x, 正解tを与えて学習させる.
    ただし, learn == 0 のときは学習を行わない.
    二乗和誤差を返す.
    */
    // 正解を予想する.
    nn_forward(nn, x, y);
    // 誤差を求める.
    double *dout;
    dout = malloc(nn->sigmoid1->len * sizeof(double));
    double res = sse(y, t, dout, nn->sigmoid1->len);
    if (learn){
        // 誤差を逆伝播させる.
        nn_backward(nn, dout);
        // パラメータを更新する.
        adam(nn->affine1, nn->v1, lr, 0.9, 0.999, 1e-7);
        adam(nn->affine2, nn->v2, lr, 0.9, 0.999, 1e-7);
        adam(nn->affine3, nn->v3, lr, 0.9, 0.999, 1e-7);
    }
    // メモリを解放する.
    free(dout);
    // 二乗和誤差を返す.
    return res;
}

int is_correct(const double y[], const double t[]){
    // モデルの出力yがtと一致しているかを返す.
    assert(t[0] == 0.0 || t[0] == 1.0);
    if (y[0] < 0.5)
        return t[0] == 0.0;
    else
        return t[0] == 1.0;
}

void nn_learn(NeuralNetwork *nn, double **X_train, double **y_train, int train_size, double lr){
    // X_train, y_trainから学習を行う.
    int correct_counter = 0;
    double sum_loss = 0.0;
    // データをシャッフルする.
    int *indices = malloc(train_size * sizeof(int));
    for (int i = 0; i < train_size; i++)
        indices[i] = i;
    shuffle(indices, train_size);
    // 予測およびパラメータの更新を行う.
    double *y;
    y = malloc(nn->sigmoid1->len *sizeof(double));
    for (int i = 0; i < train_size; i++){
        sum_loss += nn_predict(nn, X_train[indices[i]], y_train[indices[i]], y, lr, 1);
        correct_counter += is_correct(y, y_train[indices[i]]);
    }
    free(y);
    printf("--- Train Accuracy: %lf, Train Loss: %lf\n", (double)correct_counter/(double)train_size, sum_loss/(double)train_size);
}

void nn_test(NeuralNetwork *nn, double **X_test, double **y_test, int test_size){
    // X_testの予測がy_testとどの程度一致しているかを出力する.
    int correct_counter = 0;
    double sum_loss = 0.0;
    double *y;
    y = malloc(nn->sigmoid1->len *sizeof(double));
    for (int i = 0; i < test_size; i++){
        sum_loss += nn_predict(nn, X_test[i], y_test[i], y, 0.0, 0);
        correct_counter += is_correct(y, y_test[i]);
    }
    printf("--- Valid Accuracy: %lf, Valid Loss: %lf\n", (double)correct_counter/(double)test_size, sum_loss/(double)test_size);
}

void nn_fit(NeuralNetwork *nn, double **X_train, double **y_train, int train_size, double **X_test, double **y_test, int test_size, double lr, int epoch){
    // モデルの学習を行う.
    for (int i = 0; i < epoch; i++){
        printf("epoch %d\n", i+1);
        nn_learn(nn, X_train, y_train, train_size, lr);
        nn_test(nn, X_test, y_test, test_size);
    }
}

void nn_load_weights(NeuralNetwork *nn, char weights_file[]){
    // weights_fileから重みを読み込む.
    // 最適化関数の変数は読み込まない.
    FILE *fp = fopen(weights_file, "r");
    for (int i = 0; i < nn->affine1->m * nn->affine1->n; i++)
        fscanf(fp, "%lf", &nn->affine1->w[i]);
    for (int i = 0; i < nn->affine1->m; i++)
        fscanf(fp, "%lf", &nn->affine1->b[i]);
    for (int i = 0; i < nn->affine2->m * nn->affine2->n; i++)
        fscanf(fp, "%lf", &nn->affine2->w[i]);
    for (int i = 0; i < nn->affine2->m; i++)
        fscanf(fp, "%lf", &nn->affine2->b[i]);
    for (int i = 0; i < nn->affine3->m * nn->affine3->n; i++)
        fscanf(fp, "%lf", &nn->affine3->w[i]);
    for (int i = 0; i < nn->affine3->m; i++)
        fscanf(fp, "%lf", &nn->affine3->b[i]);
    fclose(fp);
    //printf("Weights are loaded.\n");
}

void nn_save_weights(NeuralNetwork *nn, char weights_file[]){
    // weights_fileに重みを書き込む.
    // 最適化関数の変数は書き込まない.
    FILE *fp = fopen(weights_file, "w");
    for (int i = 0; i < nn->affine1->m * nn->affine1->n; i++)
        fprintf(fp, "%lf ", nn->affine1->w[i]);
    fprintf(fp, "\n");
    for (int i = 0; i < nn->affine1->m; i++)
        fprintf(fp, "%lf ", nn->affine1->b[i]);
    fprintf(fp, "\n");
    for (int i = 0; i < nn->affine2->m * nn->affine2->n; i++)
        fprintf(fp, "%lf ", nn->affine2->w[i]);
    fprintf(fp, "\n");
    for (int i = 0; i < nn->affine2->m; i++)
        fprintf(fp, "%lf ", nn->affine2->b[i]);
    fprintf(fp, "\n");
    for (int i = 0; i < nn->affine3->m * nn->affine3->n; i++)
        fprintf(fp, "%lf ", nn->affine3->w[i]);
    fprintf(fp, "\n");
    for (int i = 0; i < nn->affine3->m; i++)
        fprintf(fp, "%lf ", nn->affine3->b[i]);
    fprintf(fp, "\n");
    fclose(fp);
    printf("Weights are saved.\n");
}