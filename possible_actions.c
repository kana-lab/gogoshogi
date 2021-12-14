#ifndef POSSIBLE_ACTIONS
#define POSSIBLE_ACTIONS


#include "gamedef.c"

#define TOP 0           // 駒が成れるx座標
#define LEN_ACTIONS 250 // 選択可能な指手の個数の最大値


/*
可能な指手を全て列挙するget_all_actionsおよび, それに関する関数を定義した.

get_all_actionsの流れ
・add_move_actionsで盤面上の駒を動かす指手を列挙する.
    ・この際, 特殊な反則手については考えない.
・add_promotionsで駒が成る指手を追加する.
    ・歩が成れるときに必ず成るようにする.
・add_drop_actionsで持ち駒を打つ指手を追加する.
    ・二歩を打てないようにする.
    ・歩を最上段に打てないようにする.
・盤面を更新した後に自分が王手されている指手を削除する.
・打ち歩詰めの指手を削除する.
*/


/************************************
 * 定義済み関数一覧
 *   - print_piece_moves
 *   - add_move_actions
 *   - add_promotions
 *   - get_piece_position
 *   - add_drop_actions
 *   - is_checking
 *   - is_checked
 *   - get_all_actions
 *   - get_number_of_moves
 *   - is_checkmate
 *   - is_possible_action
 *   - is_useful
 *   - get_useful_actions
 ************************************/


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


int add_move_actions(const Board *b, Action actions[LEN_ACTIONS], int end_index){
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
                    if (0 <= x && x < 5 && 0 <= y && y < 5 && b->board[x][y] <= EMPTY){
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
                    while (0 <= x && x < 5 && 0 <= y && y < 5 && b->board[x][y] <= EMPTY){
                        Action action = {0, i, j, x, y, 0};
                        actions[end_index++] = action;
                        x += move_matrix_x[piece%NARI][k];
                        y += move_matrix_y[piece%NARI][k];
                        if (b->board[x][y] < EMPTY)
                            // 相手の駒を取ったとき
                            break;
                    }
                }
            }
        }
    }
    return end_index;
}


int add_promotions(const Board *b, Action actions[LEN_ACTIONS], int start_index, int end_index){
    /*
    駒が成る指手をactionsに追加する.
    start_index <= i < end_index を満たすiについて,
    actions[i].promotion = 1 とした手を追加する.
    歩が成れるときに必ず成ることに注意する.
    返り値 = end_index + 追加した指手の個数
    */
    int old_end_index = end_index;
    for (int i = start_index; i < old_end_index; i++){
        assert(actions[i].from_stock == 0);
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


int get_piece_position(const Board *b, int *x, int *y, int piece){
    /*
    pieceを1つ見つけて, その位置を*xと*yに代入する.
    pieceがない場合にはxもyも変えない.
    */
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++){
            if (b->board[i][j] == piece){
                *x = i;
                *y = j;
                return 0;
            }
        }
    }
    if (piece == OU || piece == -OU)
        // 王がないとき.
        debug_print("error: get_piece_position: The king does not exist.");
    return -1;
}


// update_board, reverse_boardを使用
int add_drop_actions(const Board *b, Action actions[LEN_ACTIONS], int end_index){
    /*
    持ち駒を打つ指手をactionsに追加する.
    歩を最上段に打てないこと, 二歩に注意する.
    ここでは打ち歩詰めについては考えない.
    返り値 = end_index + 追加した指手の個数
    */
    int x = -1, y = -1;
    if (b->next_stock[FU] == 1)
        // 歩を打てるときに自分の負を探す.
        get_piece_position(b, &x, &y, FU);
    
    for (int i = 0; i < 5; i++){
        for (int j = 0; j < 5; j++){
            if (b->board[i][j] == EMPTY){
                int min_k;
                if (i == TOP || j == y)
                    // 最上段や自分の歩がある列には歩を打てない.
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


int is_checking(const Board *b){
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
int is_checked(Board b){
    /*
    手番側が王手されているかを判定する.
    引数のbはconstではなく, 直接編集されることに注意する.
    */
    reverse_board(&b);
    return is_checking(&b);
}


// update_boardを使用
int get_all_actions(const Board *b, Action all_actions[LEN_ACTIONS]){
    /*
    選択可能な指手を全列挙する.
    王手放置や打ち歩詰めに注意する.
    */

    // 選択可能な指手の候補を全列挙する.
    Action tmp_actions[LEN_ACTIONS];
    int len_tmp_actions = 0;
    len_tmp_actions = add_move_actions(b, tmp_actions, len_tmp_actions);
    len_tmp_actions = add_promotions(b, tmp_actions, 0, len_tmp_actions);
    len_tmp_actions = add_drop_actions(b, tmp_actions, len_tmp_actions);

    // 王手放置や打ち歩詰めにならない指手をall_actionsに追加する.
    int end_index = 0;
    for (int i = 0; i < len_tmp_actions; i++){
        Board next_b = *b;
        update_board(&next_b, tmp_actions[i]);
        if (is_checked(next_b))
            // 王手放置のとき
            continue;
        if (tmp_actions[i].from_stock == FU && b->board[tmp_actions[i].to_x+move_matrix_x[FU][0]][tmp_actions[i].to_y+move_matrix_y[FU][0]] == -OU){
            // 歩を打って王手するとき
            Board next_b = *b;
            update_board(&next_b, tmp_actions[i]);
            reverse_board(&next_b);
            Action next_actions[LEN_ACTIONS];
            if (get_all_actions(&next_b, next_actions) == 0)
                // 打ち歩詰めのとき
                continue;
        }
        all_actions[end_index++] = tmp_actions[i];
    }

    return end_index;
}


int get_number_of_moves(const Board *b){
    // 手番側の可能な指手の個数を返す.
    Action all_actions[LEN_ACTIONS];
    return get_all_actions(b, all_actions);
}


int is_checkmate(const Board *b){
    // 詰みなら1, 詰みでないなら0を返す.
    // 手番側に可能な指手があるかどうかを探す.
    return get_number_of_moves(b) == 0;
}


int is_possible_action(const Board *b, const Action *action){
    // actionが正当なら1, 不当なら0を返す.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions(b, all_actions);
    for (int i = 0; i < len_all_actions; i++){
        if (action_equal(action, &all_actions[i]))
            return 1;
    }
    return 0;
}


int is_useful(const Board *b, const Action *action){
    /*
    actionが有用かどうかを簡単に判定する.
    飛や角が成れるのに成らない指手のみに対し0を返す.
    */
    if (action->from_stock || action->promotion == 1)
        // 持ち駒を打つときや駒が成るとき
        return 1;
    if (b->board[action->from_x][action->from_y] == HISHA || b->board[action->from_x][action->from_y] == KAKU){
        // 飛や角を動かすとき
        if (action->from_x == TOP || action->to_x == TOP)
            // 駒が成れるとき
            return 0;
    }
    // それ以外のとき
    return 1;
}


// update_boardとreverse_boardを使用
int get_useful_actions(const Board *b, Action actions[LEN_ACTIONS]){
    /*
    有用な指手を列挙する.
    相手の王を詰ませられるときは, その1手を代入する.
    そうでないときは, ごく一部の無駄な指手だけを除いて, ほぼ全ての指手を列挙する.
    */

    // 選択可能な指手を全て列挙する.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions(b, all_actions);

    // 相手の王を詰ませられるかを判定する.
    for (int i = 0; i < len_all_actions; i++){
        Board next_b = *b;
        update_board(&next_b, all_actions[i]);
        reverse_board(&next_b);
        if (is_checkmate(&next_b)){
            // all_actions[i]を行うと相手の王が詰むとき
            actions[0] = all_actions[i];
            return 1;
        }
    }

    // ほぼ明らかに無駄な指手以外を列挙する.
    int end_index = 0;
    for (int i = 0; i < len_all_actions; i++){
        if (is_useful(b, &all_actions[i]))
            actions[end_index++] = all_actions[i];
    }

    return end_index;
}


#endif  /* POSSIBLE_ACTIONS */

/*
int main(void){
    print_piece_moves();
    return 0;
}
*/
