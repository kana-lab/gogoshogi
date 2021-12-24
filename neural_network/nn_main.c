#include "dataset_creator.c"

#define MODEL_FILE "nn_585_32x2_1.txt"


int main(void){
    
    self_match_learning(MODEL_FILE, 100);

    return 0;
}
