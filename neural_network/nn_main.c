#include "dataset_creator.c"

#define MODEL_FILE "nn_361_32x2_0.txt"
#define DATASET "checkmates456554.txt"


int main(void){
    
    learn_dataset(DATASET, 360000, 96554, NULL, MODEL_FILE);
    //self_match_learning(MODEL_FILE, 1);
    //learn_dataset(DATASET, 0, 100000, MODEL_FILE, NULL);

    return 0;
}
