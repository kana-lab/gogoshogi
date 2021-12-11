#include "gamedef.c"

#include <assert.h>

#define TOP 0           // 駒が成れるx座標
#define LEN_ACTIONS 300 // 選択可能な指手の個数の最大値


int move_matrix_x[MAX_PIECE_NUMBER+1][8] = {
    {},                   // EMPTY
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

int move_matrix_y[MAX_PIECE_NUMBER+1][8] = {
    {},                   // EMPTY
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

int move_length[MAX_PIECE_NUMBER+1] = {0, 1, 4, 4, 5, 6, 8, 6, 4, 4, 6};


void print_piece_moves(void){
    /*
    それぞれの駒について可能な動きを表示する.
    ただし, 馬や龍については, 成ることによって増えた動きのみを表示する.
    */
    char *piece_ch[] = {
        "..",
        "FU", "KK", "HI", "GI", "KI", "OU",
        "TO", "UM", "RY", "ZE"
    };
    printf("\nmoves of pieces\n");
    for (int piece = 1; piece <= MAX_PIECE_NUMBER; piece++){
        char grid[3][3][3];
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++)
                strcpy(grid[i][j], piece_ch[EMPTY]);
        }
        strcpy(grid[1][1], piece_ch[piece]);
        for (int i = 0; i < move_length[piece]; i++){
            strcpy(grid[1+move_matrix_x[piece][i]][1+move_matrix_y[piece][i]], "**");
        }
        for (int i = 0; i < 3; i++){
            putchar('\n');
            for (int j = 0; j < 3; j++){
                printf("%s", grid[i][j]);
            }
        }
        putchar('\n');
    }
    putchar('\n');
}


int add_move_actions(Board *b, Action *actions, int end_index){
    /*
    盤面上の駒を動かす指手をactionsに追加する.
    actions[end_index]から順に, Action型の指手を代入する.
    駒は全て成らないものとする.
    返り値 = end_index + 追加した指手の個数
    */
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++){
            int piece = b->board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            if (piece != KAKU && piece != HISHA){
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する.
                for (int k = 0; k < move_length[piece]; k++){
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    if (0 <= x && x < 5 && 0 <= y && y < 5 && b->board[x][y] <= 0){
                        Action action = {0, i, j, x, y, 0};
                        actions[end_index++] = action;
                    }
                }
            }
            if (piece % NARI == KAKU || piece % NARI == HISHA){
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する.
                for (int k = 0; k < move_length[piece%NARI]; k++){
                    int x = i + move_matrix_x[piece%NARI][k];
                    int y = j + move_matrix_y[piece%NARI][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5 && b->board[x][y] <= 0){
                        Action action = {0, i, j, x, y, 0};
                        actions[end_index++] = action;
                        x += move_matrix_x[piece%NARI][k];
                        y += move_matrix_y[piece%NARI][k];
                    }
                }
            }
        }
    }
    return end_index;
}


int add_promotions(Board *b, Action *actions, int start_index, int end_index){
    /*
    駒が成る指手をactionsに追加する.
    start_index <= i < end_index を満たすiについて,
    actions[i].promotion = 1 とした手を追加する.
    返り値 = end_index + 追加した指手の個数
    */
    int old_end_index = end_index;
    for (int i = start_index; i < old_end_index; i++){
        assert(actions[i].promotion == 0);
        int piece = b->board[actions[i].from_x][actions[i].from_y];
        if (piece <= GIN && (actions[i].to_x == TOP || actions[i].from_x == TOP)){
            if (piece == FU)
                // 歩が成れるときは必ず成る.
                actions[i].promotion = 1;
            else{
                // 角, 飛, 銀が成る手を追加する.
                actions[end_index] = actions[i];
                actions[end_index++].promotion = 1;
            }
        }
    }
    return end_index;
}


int add_drop_actions(Board *b, Action *actions, int end_index){
    /*
    駒を打つ指手をactionsに追加する.
    返り値 = end_index + 追加した指手の個数
    */
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++){
            if (b->board[i][j] == EMPTY){
                int min_k;
                if (i == TOP)
                    // 敵陣には歩を打てない.
                    min_k = 2;
                else
                    min_k = 1;
                for (int k = min_k; k < 6; k++){
                    if (b->next_stock[k]){
                        Action action = {k, -1, -1, i, j};
                        actions[end_index++] = action;
                    }
                }
            }
        }
    }
    return end_index;
}


int get_piece_position(Board *b, int *x, int *y, int piece){
    /*
    pieceを1つ見つけて, その位置を*xと*yに代入する.
    基本的には piece = OU,-OU の形で使用する.
    */
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++){
            if (b->board[i][j] == -OU){
                *x = i;
                *y = j;
                return 0;
            }
        }
    }
    print_debug("error: get_piece_position: The piece does not exist.");
    return -1;
}


int is_checking(Board *b){
    // 手番側が王手しているかを判定する.

    // 可能な駒移動の指手を列挙する.
    Action all_actions[LEN_ACTIONS];
    int end_index = add_move_actions(b, all_actions, 0);

    // 相手の王の位置を取得する.
    int x, y;
    get_piece_position(b, &x, &y, -OU);

    // 相手の王をとれる状態かを判定する.
    for (int i = 0; i < end_index; i++){
        if (all_actions[i].to_x == x && all_actions[i].to_y == y)
            return 1;
    }
    return 0;
}


// reverse_boardを使用
int is_checked(Board *b){
    // 手番側が王手されているかを判定する.
    return is_checking(reverse_board(b));
}


// update_boardを使用
int get_all_actions(Board *b, Action *all_actions){
    // 選択可能な指手を全列挙する.

    // 選択可能な指手の候補を全列挙する.
    Action tmp_actions[LEN_ACTIONS];
    int len_tmp_actions = 0;
    len_tmp_actions = add_move_actions(b, tmp_actions, len_tmp_actions);
    len_tmp_actions = add_promotions(b, tmp_actions, 0, len_tmp_actions);
    len_tmp_actions = add_drop_actions(b, tmp_actions, len_tmp_actions);

    // 王手放置にならない指手をall_actionsに追加する.
    int end_index = 0;
    for (int i = 0; i < len_tmp_actions; i++){
        Board next_b = update_board(b, &tmp_actions[i]);
        if (is_checked(&next_b) == 0)
            // 王手放置でないとき
            all_actions[end_index++] = tmp_actions[i];
    }

    return end_index;
}

