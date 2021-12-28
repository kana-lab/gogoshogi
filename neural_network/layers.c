#include "mathmatical_functions.c"


/*
ニューラルネットワークの層および最適化関数を実装した.
*/


// ------ layers ------

// ReLU layer

typedef struct{
    int len;     // 入出力ノード数
    double *out; // 出力
}ReluLayer;

void relu_init(ReluLayer *layer, int len){
    // ReluLayerを初期化する.
    layer->len = len;
    layer->out = malloc(len * sizeof(double));
}

void relu_free(ReluLayer *layer){
    // ReluLayerに割り当てたメモリを解放する.
    free(layer->out);
}

void relu_forward(ReluLayer *layer, const double x[]){
    // 入力xを受け取って, outに結果を出力する.
    // x -- ReluLayer --> out
    for (int i = 0; i < layer->len; i++){
        if (x[i] <= 0.0){
            layer->out[i] = 0.0;
        }else{
            layer->out[i] = x[i];
        }
    }
}

void relu_backward(const ReluLayer *layer, const double dout[], double dx[]){
    // 勾配doutを受け取って, dxに結果を出力する.
    // dx <-- ReluLayer -- dout
    for (int i = 0; i < layer->len; i++){
        if (layer->out[i] == 0.0)
            dx[i] = 0.0;
        else
            dx[i] = dout[i];
    }
}


// sigmoid layer

typedef struct{
    int len;     // 入出力ノード数
    double *out; // 出力結果
    double *dout;// 逆伝播の入力
}SigmoidLayer;

void sigmoid_init(SigmoidLayer *layer, int len){
    // SigmoidLayerを初期化する.
    layer->len = len;
    layer->out = malloc(len * sizeof(double));
    layer->dout = malloc(len * sizeof(double));
}

void sigmoid_free(SigmoidLayer *layer){
    // SigmoidLayerに割り当てたメモリを解放する.
    free(layer->out);
    free(layer->dout);
}

void sigmoid_forward(SigmoidLayer *layer, const double x[]){
    // 入力xを受け取って, outに結果を出力する.
    // x -- SigmoidLayer --> out
    for (int i = 0; i < layer->len; i++){
        layer->out[i] = 1.0 / (1.0 + pow(M_E, -x[i]));
    }
}

void sigmoid_backward(const SigmoidLayer *layer, double dx[]){
    // 勾配doutを受け取って, dxに結果を出力する.
    // 勾配消失を防ぐため, 微分をせずにそのまま上流に流している.
    // dx <-- SigmoidLayer -- dout
    for (int i = 0; i < layer->len; i++)
        dx[i] = layer->dout[i];// * (1.0 - layer->out[i]) * layer->out[i];
}


// affine layer

typedef struct{
    int n;       // 入力ノード数
    int m;       // 出力ノード数
    double *w;   // weights(m x n)
    double *b;   // bias(m)
    double *x;   // 順伝播の入力, 逆伝播の出力dxと兼用
    double *out; // 順伝播の出力, 逆伝播の入力doutと兼用
    double *dw;  // wの微分
    double *db;  // bの微分
    double *dx;  // xの微分
}AffineLayer;

void affine_init(AffineLayer *layer, int n, int m){
    // AffineLayerを初期化する.
    // 入力ノード数はn, 出力ノード数はmである.
    layer->n = n;
    layer->m = m;
    layer->w = malloc(m*n * sizeof(double));
    layer->b = malloc(m * sizeof(double));
    layer->x = malloc(n * sizeof(double));
    layer->out = malloc(m * sizeof(double));
    layer->dw = malloc(m*n * sizeof(double));
    layer->db = malloc(m * sizeof(double));
}

void affine_free(AffineLayer *layer){
    // AffineLayerに割り当てたメモリを解放する.
    free(layer->w);
    free(layer->b);
    free(layer->x);
    free(layer->out);
    free(layer->dw);
    free(layer->db);
}

void affine_init_with_xavier(AffineLayer *layer, int n, int m){
    // Xavierの初期値でAffineLayerを初期化する.
    affine_init(layer, n, m);
    for (int i = 0; i < m; i++){
        for (int j = 0; j < n; j++)
            layer->w[n*i+j] = xavier(n);
    }
}

void affine_init_with_he(AffineLayer *layer, int n, int m){
    // Heの初期値でAffineLayerを初期化する.
    affine_init(layer, n, m);
    for (int i = 0; i < m; i++){
        for (int j = 0; j < n; j++)
            layer->w[n*i+j] = he(n);
    }
}

void affine_forward(AffineLayer *layer, const double x[]){
    // 入力xを受け取って, outに結果を出力する.
    // x -- AffineLayer --> out
    for (int i = 0; i < layer->n; i++)
        layer->x[i] = x[i];
    mat_mul_vec(layer->w, x, layer->out, layer->m, layer->n);
}

void affine_backward(AffineLayer *layer){
    // 勾配doutを受け取って, dxに結果を出力する.
    // dx(x) <-- AffineLayer -- dout(out)
    // dwを計算する.
    vec_mul_vecT(layer->out, layer->x, layer->dw, layer->m, layer->n);
    // dxを計算する.
    matT_mul_vec(layer->w, layer->out, layer->x, layer->n, layer->m);
    // dbを計算する.
    for (int i = 0; i < layer->m; i++)
        layer->db[i] += layer->out[i];
}

void affine_clear_d(AffineLayer *layer){
    // 勾配を0.0で初期化する.
    for (int i = 0; i < layer->m*layer->n; i++){
        layer->dw[i] = 0.0;
    }
    for (int i = 0; i < layer->m; i++){
        layer->db[i] = 0.0;
    }
}


// ------ optimizers ------

typedef struct{
    // 最適化関数で用いられる変数の定義
    double *wv;
    double *bv;
    double *ws;
    double *bs;
}Velocities;

void velocities_init(Velocities *v, const AffineLayer *layer){
    // Velocitiesを初期化する.
    v->wv = calloc(layer->m*layer->n, sizeof(double));
    v->bv = calloc(layer->m, sizeof(double));
    v->ws = calloc(layer->m*layer->n, sizeof(double));
    v->bs = calloc(layer->m, sizeof(double));
}

void velocities_free(Velocities *v){
    // Velocitiesに割り当てたメモリを解放する.
    free(v->wv);
    free(v->bv);
    free(v->ws);
    free(v->bs);
}


// SGD

void sgd(AffineLayer *layer, double lr){
    // SGDによってAffineLayerのパラメータを更新する.
    // 正確にはSGDではなく最急降下法になっているかもしれない.
    // パラメータの目安: lr=0.01
    assert(0.0 < lr && lr <= 0.1);
    // wの更新
    for (int i = 0; i < layer->m; i++){
        for (int j = 0; j < layer->n; j++)
            layer->w[layer->n*i+j] -= lr * layer->dw[layer->n*i+j];
    }
    // bの更新
    for (int i = 0; i < layer->m; i++)
        layer->b[i] -= lr * layer->db[i];
}


// Momentum

void momentum(AffineLayer *layer, Velocities *v, double lr, double beta){
    // MomentumによってAffineLayerのパラメータを更新する.
    // パラメータの目安: lr=0.01, beta=0.9
    assert(0.0 < lr && lr <= 0.1);
    assert(0.0 < beta && beta < 1.0);
    // wの更新
    for (int i = 0; i < layer->m; i++){
        for (int j = 0; j < layer->n; j++){
            v->wv[layer->n*i+j] = beta*v->wv[layer->n*i+j] + (1.0-beta)*layer->dw[layer->n*i+j];
            layer->w[layer->n*i+j] -= lr * v->wv[layer->n*i+j];
        }
    }
    // bの更新
    for (int i = 0; i < layer->m; i++){
        v->bv[i] = beta*v->bv[i] + (1.0-beta)*layer->db[i];
        layer->b[i] -= lr * v->bv[i];
    }
}


// RMSProp

void rmsprop(AffineLayer *layer, Velocities *v, double lr, double beta, double epsilon){
    // RMSPropによってAffineLayerのパラメータを更新する.
    // パラメータの目安: lr=0.01, beta=0.9, epsilon=1e-7
    assert(0.0 < lr && lr <= 0.1);
    assert(0.0 < beta && beta < 1.0);
    assert(0.0 < epsilon && epsilon <= 0.01);
    // wの更新
    for (int i = 0; i < layer->m; i++){
        for (int j = 0; j < layer->n; j++){
            v->wv[layer->n*i+j] = beta*v->wv[layer->n*i+j] + (1.0-beta)*layer->dw[layer->n*i+j]*layer->dw[layer->n*i+j];
            layer->w[layer->n*i+j] -= lr * layer->dw[layer->n*i+j] / sqrt(v->wv[layer->n*i+j]+epsilon);
        }
    }
    // bの更新
    for (int i = 0; i < layer->m; i++){
        v->bv[i] = beta*v->bv[i] + (1.0-beta)*layer->db[i]*layer->db[i];
        layer->b[i] -= lr * layer->db[i] / sqrt(v->bv[i]+epsilon);
    }
}


// Adam

void adam(AffineLayer *layer, Velocities *v, double lr, double beta1, double beta2, double epsilon){
    // AdamによってAffineLayerのパラメータを更新する.
    // パラメータの目安: lr=0.01, beta1=0.9, beta2=0.999, epsilon=1e-7
    assert(0.0 < lr && lr <= 0.1);
    assert(0.0 < beta1 && beta1 < 1.0);
    assert(0.0 < beta2 && beta2 < 1.0);
    assert(0.0 < epsilon && epsilon <= 0.01);
    // wの更新
    for (int i = 0; i < layer->m; i++){
        for (int j = 0; j < layer->n; j++){
            v->wv[layer->n*i+j] = beta1*v->wv[layer->n*i+j] + (1.0-beta1)*layer->dw[layer->n*i+j];
            v->ws[layer->n*i+j] = beta2*v->ws[layer->n*i+j] + (1.0-beta2)*layer->dw[layer->n*i+j]*layer->dw[layer->n*i+j];
            layer->w[layer->n*i+j] -= lr * v->wv[layer->n*i+j] / sqrt(v->ws[layer->n*i+j]+epsilon);
        }
    }
    // bの更新
    for (int i = 0; i < layer->m; i++){
        v->bv[i] = beta1*v->bv[i] + (1.0-beta1)*layer->db[i];
        v->bs[i] = beta2*v->bs[i] + (1.0-beta2)*layer->db[i]*layer->db[i];
        layer->b[i] -= lr * v->bv[i] / sqrt(v->bs[i]+epsilon);
    }
}
