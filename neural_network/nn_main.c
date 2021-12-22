#include "dataset_creator.c"

#define DATASET "initial_boards.txt"


int main(void){

    // プレイヤーを初期化する.
    AI first = create_random_move_ai();
    AI second = create_random_move_ai();

    // データセットを作成する.
    create_dataset((PlayerInterface *) &first, (PlayerInterface *) &second, DATASET, 10);

    return 0;
}
