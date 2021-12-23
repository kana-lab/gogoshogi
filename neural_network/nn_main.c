#include "dataset_creator.c"

#define DATASET "self_match.txt"


int main(void){
    
    // プレイヤーを初期化する.
    NNAI first = create_read1_ai("nn_585_32x2_0.txt");
    NNAI second = create_read1_ai("nn_585_32x2_0.txt");

    // データセットを作成する.
    create_dataset((PlayerInterface *) &first, (PlayerInterface *) &second, DATASET, 1, "self_match");

    return 0;
}
