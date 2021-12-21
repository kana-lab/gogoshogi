#include "layers.c"

#include <stdio.h>
#include <time.h>


/*
ニューラルネットワークを実装した.
*/


typedef struct{
    // Neural Network
    // Affine[0] -> ReLU[0] -> ... -> ReLU[depth-2] -> Affine[depth-1] -> Sigmoid
    int depth;
    AffineLayer *affine;
    Velocities *velocities;
    ReluLayer *relu;
    SigmoidLayer sigmoid;
}NeuralNetwork;


void nn_init(NeuralNetwork *nn, int depth, int sizes[depth+1]){
    // NeuralNetworkを初期化する.

    // メモリを確保する.
    nn->depth = depth;
    nn->affine = malloc(depth * sizeof(AffineLayer));
    nn->velocities = malloc(depth * sizeof(Velocities));
    nn->relu = malloc((depth-1) * sizeof(ReluLayer));

    // 各層を適切な値で初期化する.
    for (int i = 0; i < depth-1; i++){
        affine_init_with_he(&nn->affine[i], sizes[i], sizes[i+1]);
        velocities_init(&nn->velocities[i], &nn->affine[i]);
        relu_init(&nn->relu[i], sizes[i+1]);
    }
    affine_init_with_xavier(&nn->affine[depth-1], sizes[depth-1], sizes[depth]);
    velocities_init(&nn->velocities[depth-1], &nn->affine[depth-1]);
    sigmoid_init(&nn->sigmoid, sizes[depth]);
}


void nn_free(NeuralNetwork *nn){
    // NeuralNetworkに割り当てたメモリを解放する.
    for (int i = 0; i < nn->depth-1; i++){
        affine_free(&nn->affine[i]);
        velocities_free(&nn->velocities[i]);
        relu_free(&nn->relu[i]);
    }
    affine_free(&nn->affine[nn->depth-1]);
    velocities_free(&nn->velocities[nn->depth-1]);
    sigmoid_free(&nn->sigmoid);
}


void nn_forward(NeuralNetwork *nn, const double x[]){
    // NeuralNetworkに入力xを与え, 出力をnn->sigmoid.outに代入する.
    affine_forward(&nn->affine[0], x);
    for (int i = 0; i < nn->depth-1; i++){
        relu_forward(&nn->relu[i], nn->affine[i].out);
        affine_forward(&nn->affine[i+1], nn->relu[i].out);
    }
    sigmoid_forward(&nn->sigmoid, nn->affine[nn->depth-1].out);
}


void nn_backward(NeuralNetwork *nn){
    // 誤差を逆伝播させる.
    // 誤差はnn->sigmoid.doutに入力してあるものとする.
    sigmoid_backward(&nn->sigmoid, nn->affine[nn->depth-1].out);
    affine_backward(&nn->affine[nn->depth-1]);
    for (int i = nn->depth-2; 0 <= i; i--){
        relu_backward(&nn->relu[i], nn->affine[i+1].x, nn->affine[i].out);
        affine_backward(&nn->affine[i]);
    }
}


double nn_predict(NeuralNetwork *nn, const double x[], const double y[], double lr){
    /*
    NeuralNetworkに1つの入力を与えて学習させる.
    入力がx, 正解がyになるようにする.
    ただし, y == NULL のときは予測のみを行う.
    また, lr == 0.0 のときは誤差逆伝播を行わない.
    二乗和誤差を返す.
    */
    
    // 正解を予想する.
    nn_forward(nn, x);

    if (y == NULL)
        // 答えがないとき
        return 0.0;
    
    // 誤差を求める.
    double res = sse(nn->sigmoid.out, y, nn->sigmoid.dout, nn->sigmoid.len);

    if (0.0 < lr){
        // lrが正のときに誤差を逆伝播させる.
        nn_backward(nn);
        // パラメータを更新する.
        for (int i = 0; i < nn->depth; i++)
            adam(&nn->affine[i], &nn->velocities[i], lr, 0.9, 0.999, 1e-7);
    }

    // 二乗和誤差を返す.
    return res;
}


int is_correct(const double y[], const double t[]){
    // モデルの出力yがtと一致しているかを返す.
    return (y[0] < 0.5) == (t[0] < 0.5);
}


void nn_train(NeuralNetwork *nn, double **X_train, double **y_train, int train_size, double lr){
    // X_train, y_trainから学習を行う.

    // 変数を初期化する.
    double correct_counter = 0.0;
    double sum_loss = 0.0;

    // データをシャッフルする.
    int *indices = malloc(train_size * sizeof(int));
    for (int i = 0; i < train_size; i++)
        indices[i] = i;
    shuffle(indices, train_size);

    // 時間を計測する.
    time_t start_time = time(NULL);

    // 予測およびパラメータの更新を行う.
    for (int i = 0; i < train_size; i++){
        sum_loss += nn_predict(nn, X_train[indices[i]], y_train[indices[i]], lr);
        correct_counter += is_correct(nn->sigmoid.out, y_train[indices[i]]);

        // 時間を出力する.
        if (i == train_size/100)
            printf("---   1%% %lds\n", time(NULL) - start_time);
        else if (i == train_size/10)
            printf("---  10%% %lds\n", time(NULL) - start_time);
        else if (i == train_size - 1)
            printf("--- 100%% %lds\n", time(NULL) - start_time);
    }

    // 結果を出力する.
    printf("--- Train Accuracy: %lf, Train Loss: %lf\n", correct_counter/(double)train_size, sum_loss/(double)train_size);
}


void nn_test(NeuralNetwork *nn, double **X_test, double **y_test, int test_size){
    // X_testの予測がy_testと一致している割合を出力する.

    // 変数を初期化する.
    double correct_counter = 0.0;
    double sum_loss = 0.0;

    // 時間を計測する.
    time_t start_time = time(NULL);

    for (int i = 0; i < test_size; i++){
        sum_loss += nn_predict(nn, X_test[i], y_test[i], 0.0);
        correct_counter += is_correct(nn->sigmoid.out, y_test[i]);

        // 時間を出力する.
        if (i == test_size/100)
            printf("---   1%% %lds\n", time(NULL) - start_time);
        else if (i == test_size/10)
            printf("---  10%% %lds\n; ", time(NULL) - start_time);
        else if (i == test_size - 1)
            printf("--- 100%% %lds\n", time(NULL) - start_time);
    }

    // 結果を出力する.
    printf("--- Test  Accuracy: %lf, Test  Loss: %lf\n", correct_counter/(double)test_size, sum_loss/(double)test_size);
}


void nn_fit(NeuralNetwork *nn, double **X_train, double **y_train, int train_size, double **X_test, double **y_test, int test_size, double lr, int epoch){
    // モデルの学習および検証を行う.
    for (int i = 0; i < epoch; i++){
        printf("epoch %d/%d\n", i+1, epoch);
        nn_train(nn, X_train, y_train, train_size, lr);
        nn_test(nn, X_test, y_test, test_size);
    }
}


void nn_load_weights(NeuralNetwork *nn, char weights_file[]){
    // weights_fileから重みを読み込む.
    // 最適化関数の変数は読み込まない.
    FILE *fp = fopen(weights_file, "r");
    for (int i = 0; i < nn->depth; i++){
        for (int j = 0; j < nn->affine[i].m * nn->affine[i].n; j++)
            fscanf(fp, "%lf", &nn->affine[i].w[j]);
        for (int j = 0; j < nn->affine[i].m; j++)
            fscanf(fp, "%lf", &nn->affine[i].b[j]);
    }
    fclose(fp);
}


void nn_save_weights(NeuralNetwork *nn, char weights_file[]){
    // weights_fileに重みを書き込む.
    // 最適化関数の変数は書き込まない.
    FILE *fp = fopen(weights_file, "w");
    for (int i = 0; i < nn->depth; i++){
        for (int j = 0; j < nn->affine[i].m * nn->affine[i].n; j++)
            fprintf(fp, "%lf ", nn->affine[i].w[j]);
        fprintf(fp, "\n");
        for (int j = 0; j < nn->affine[i].m; j++)
            fprintf(fp, "%lf ", nn->affine[i].b[j]);
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("All weights are saved.\n");
}
