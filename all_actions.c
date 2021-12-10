#include "gamedef.c"
#define TOP 0

int move_matrix_x[10][8] = {
    {-1},                 // 歩
    {-1,-1,1,1},          // 角1
    {-1,1,0,0},           // 飛1
    {-1,-1,-1,1,1},       // 銀
    {-1,-1,-1,0,0,1},     // 金
    {-1,-1,-1,0,0,1,1,1}, // 王
    {-1,-1,-1,0,0,1},     // と
    {-1,1,0,0},           // 馬 - 角
    {-1,-1,1,1},          // 龍 - 飛
    {-1,-1,-1,0,0,1}      // 全
};

int move_matrix_y[10][8] = {
    {0},                  // 歩
    {-1,1,-1,1},          // 角1
    {0,0,-1,1},           // 飛1
    {-1,0,1,-1,1},        // 銀
    {-1,0,1,-1,1,0},      // 金
    {-1,0,1,-1,1,-1,0,1}, // 王
    {-1,0,1,-1,1,0},      // と
    {0,0,-1,1},           // 馬 - 角
    {-1,1,-1,1},          // 龍 - 飛
    {-1,0,1,-1,1,0},      // 全
};

int move_length[10] = {1, 4, 4, 5, 6, 8, 6, 4, 4, 6};


void print_piece_moves(void){
    printf("the moves of pieces\n");


}


int get_all_actions(const Board b, Action *all_actions){
    int all_actions_index = 0;

    // 盤面上の駒を動かす手を列挙
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++){
            int piece = b.board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            int left = all_actions_index;
            if (piece != KAKU && piece != HISHA){
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する
                for (int k = 0; k < move_length[piece]; k++){
                    Action action;
                    action.from_stock = 0;
                    action.from_x = i;
                    action.from_y = j;
                    action.to_x = i + move_matrix_x[piece][k];
                    action.to_y = j + move_matrix_y[piece][k];
                    action.turn_over = 0; // 暫定的に駒は成らないものとする
                    all_actions[all_actions_index++] = action;
                }
            }
            if (piece % NARI == KAKU || piece & NARI == HISHA){
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する
                for (int k = 0; k < move_length[piece]; k++){
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5 && b.board[x][y] <= 0){
                        Action action;
                        action.from_stock = 0;
                        action.from_x = i;
                        action.from_y = j;
                        action.to_x = x;
                        action.to_y = y;
                        action.turn_over = 0; // 暫定的に駒は成らないものとする
                        all_actions[all_actions_index++] = action;
                        x += move_matrix_x[piece][k];
                        y += move_matrix_y[piece][k];
                    }
                }
            }
            if (KIN <= piece)
                // 駒が成れないとき
                continue;
            // 駒が成る手を追加する
            int right = all_actions_index;
            for (int k = left; k < right; k++){
                if (all_actions[k].to_x == TOP || all_actions[k].from_x == TOP){
                    if (piece == FU)
                        // 歩が成れるときは必ず成る
                        all_actions[k].turn_over = 1;
                    else{
                        // 角, 飛, 銀が成る手を追加する
                        all_actions[all_actions_index] = all_actions[k];
                        all_actions[all_actions_index++].turn_over = 1;
                    }
                }
            }
        }
    }

    return all_actions_index;
}
