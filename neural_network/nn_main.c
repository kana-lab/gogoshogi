#include "dataset_creator.c"
#include "league_match.c"

#define MODEL_FILE "nn_128x2_64x2_32x2_1.txt"
#define DATASET "checkmates456554.txt"


int main(void){
    
    //learn_dataset(DATASET, 360000, 96554, NULL, MODEL_FILE);
    //srand(1);
    //self_match_learning(MODEL_FILE, 500);
    //learn_dataset(DATASET, 0, 100000, MODEL_FILE, NULL);

    league_match();

    return 0;
}
