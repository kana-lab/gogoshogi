#include "Board.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


Board create_board(void) {
    // Board型の盤面を作って初期化し、それを戻り値として返す
    // first_mover引数は先手を表し、USER か AI のいずれかである

    Board b;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            b.board[i][j] = EMPTY;
        }
    }

    b.board[0][0] = -HISHA;
    b.board[0][1] = -KAKU;
    b.board[0][2] = -GIN;
    b.board[0][3] = -KIN;
    b.board[0][4] = -OU;
    b.board[1][4] = -FU;
    b.board[3][0] = FU;
    b.board[4][0] = OU;
    b.board[4][1] = KIN;
    b.board[4][2] = GIN;
    b.board[4][3] = KAKU;
    b.board[4][4] = HISHA;

    for (int i = 0; i < 6; i++) {
        b.next_stock[i] = b.previous_stock[i] = 0;
    }

    return b;
}


void print_board_for_debug(const Board *b) {
    // 盤面をプリントするデバッグ用の関数
    // "全"は成銀を、赤字は相手の駒を表す

#ifdef DEBUG_MODE
    static const char *piece_ch[] = {
            "・",
            "歩", "角", "飛", "銀", "金", "王",
            "と", "馬", "龍", "全"
    };

    // 盤面のプリント
    puts("  ＡＢＣＤＥ");
    for (int i = 0; i < 5; ++i) {
        printf("%d ", 5 - i);
        for (int j = 0; j < 5; ++j) {
            int piece = b->board[i][j];

            // ユーザーの駒かAIの駒かによって色を変える
            if (piece < 0) {
                printf("\x1b[31m");
            } else {
                printf("\x1b[39m");
            }

            printf("%s", piece_ch[abs(piece)]);
        }
        printf("\x1b[39m\n");
    }

    // ユーザーの持ち駒のプリント
    puts("");
    printf("me      : ");
    for (int piece = 0; piece < 6; ++piece) {
        int count = b->next_stock[piece];
        for (int i = 0; i < count; ++i)
            printf("%s", piece_ch[piece]);
    }

    // AIの持ち駒のプリント
    puts("");
    printf("opponent: ");
    for (int piece = 0; piece < 6; ++piece) {
        int count = b->previous_stock[piece];
        for (int i = 0; i < count; ++i)
            printf("%s", piece_ch[piece]);
    }

    printf("\n\n");
#endif  /* DEBUG_MODE */
}


bool board_equal(const Board *b1, const Board *b2) {
    // 2つの盤面が等しければ1を、等しくなければ0を返す
    // 次に打つプレイヤーが誰かは考慮しない

    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            if (b1->board[i][j] != b2->board[i][j])
                return false;

    for (int i = 0; i < 6; ++i) {
        if (b1->next_stock[i] != b2->next_stock[i])
            return false;
        if (b1->previous_stock[i] != b2->previous_stock[i])
            return false;
    }

    return true;
}


void reverse_board(Board *b) {
    // 盤面を反転させる
    // b.board[5][5]の全要素に-1を掛けて180°回転する

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            b->board[i][j] *= -1;
        }
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            if (j == 2 && i >= 3) continue;
            else {
                int temp = b->board[i][j];
                b->board[i][j] = b->board[4 - i][4 - j];
                b->board[4 - i][4 - j] = temp;
            }
        }
    }
    for (int i = 0; i < 6; i++) {
        int temp = b->previous_stock[i];
        b->previous_stock[i] = b->next_stock[i];
        b->next_stock[i] = temp;
    }
}


void update_board(Board *b, Action action) {
    // 駒を動かして盤面を更新する
    // 一切の反則手のチェックをしないので注意！
    // インデックスの範囲のチェックぐらいはするかも (しない)

    if (action.from_stock) {  // 持ち駒を打つ場合
        b->board[action.to_x][action.to_y] = action.from_stock;
        --b->next_stock[action.from_stock];
    } else {  // 駒を動かす場合
        int piece = b->board[action.from_x][action.from_y];
        b->board[action.from_x][action.from_y] = EMPTY;  // 移動元を空にする

        int gain = b->board[action.to_x][action.to_y];
        if (gain != EMPTY)  // 移動先に相手の駒がある場合、それを持ち駒に加える
            ++b->next_stock[abs(gain) % NARI];

        if (action.promotion && piece < NARI)
            piece += NARI;  // 駒を成る指示があれば NARI を加える

        b->board[action.to_x][action.to_y] = piece;  // 駒を移動先に持っていく
    }
}


static int move_matrix_x[MAX_PIECE_NUMBER + 1][8] = {
        {},                           // EMPTY
        {-1},                         // 歩
        {-1, -1, 1,  1},              // 角1
        {-1, 1,  0,  0},              // 飛1
        {-1, -1, -1, 1, 1},           // 銀
        {-1, -1, -1, 0, 0, 1},        // 金
        {-1, -1, -1, 0, 0, 1, 1, 1},  // 王
        {-1, -1, -1, 0, 0, 1},        // と
        {-1, 1,  0,  0},              // 馬 - 角
        {-1, -1, 1,  1},              // 龍 - 飛
        {-1, -1, -1, 0, 0, 1}         // 全
};

static int move_matrix_y[MAX_PIECE_NUMBER + 1][8] = {
        {},                            // EMPTY
        {0},                           // 歩
        {-1, 1, -1, 1},                // 角1
        {0,  0, -1, 1},                // 飛1
        {-1, 0, 1,  -1, 1},            // 銀
        {-1, 0, 1,  -1, 1, 0},         // 金
        {-1, 0, 1,  -1, 1, -1, 0, 1},  // 王
        {-1, 0, 1,  -1, 1, 0},         // と
        {0,  0, -1, 1},                // 馬 - 角
        {-1, 1, -1, 1},                // 龍 - 飛
        {-1, 0, 1,  -1, 1, 0},         // 全
};

static int move_length[MAX_PIECE_NUMBER + 1] = {0, 1, 4, 4, 5, 6, 8, 6, 4, 4, 6};


static void print_piece_moves(void) {
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
    for (int piece = 1; piece <= MAX_PIECE_NUMBER; piece++) {
        char grid[3][3][3];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++)
                strcpy(grid[i][j], piece_ch[EMPTY]);
        }
        strcpy(grid[1][1], piece_ch[piece]);
        for (int i = 0; i < move_length[piece]; i++) {
            strcpy(grid[1 + move_matrix_x[piece][i]][1 + move_matrix_y[piece][i]], "**");
        }
        for (int i = 0; i < 3; i++) {
            putchar('\n');
            for (int j = 0; j < 3; j++) {
                printf("%s", grid[i][j]);
            }
        }
        putchar('\n');
    }
    putchar('\n');
}


static int add_move_actions(const Board *b, Action actions[LEN_ACTIONS], int end_index) {
    /*
    盤面上の駒を動かす指手をactionsに追加する.
    actions[end_index]から順に, Action型の指手を代入する.
    駒は全て成らないものとする.
    返り値 = end_index + 追加した指手の個数
    */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = b->board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            if (piece != KAKU && piece != HISHA) {
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する.
                for (int k = 0; k < move_length[piece]; k++) {
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    if (0 <= x && x < 5 && 0 <= y && y < 5 && b->board[x][y] <= EMPTY) {
                        Action action = {0, i, j, x, y, 0};
                        actions[end_index++] = action;
                    }
                }
            }
            if (piece % NARI == KAKU || piece % NARI == HISHA) {
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する.
                for (int k = 0; k < move_length[piece % NARI]; k++) {
                    int x = i + move_matrix_x[piece % NARI][k];
                    int y = j + move_matrix_y[piece % NARI][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5 && b->board[x][y] <= EMPTY) {
                        Action action = {0, i, j, x, y, 0};
                        actions[end_index++] = action;
                        if (b->board[x][y] < EMPTY)
                            // 相手の駒を取ったとき
                            break;
                        x += move_matrix_x[piece % NARI][k];
                        y += move_matrix_y[piece % NARI][k];

                    }
                }
            }
        }
    }
    return end_index;
}


static int add_promotions(const Board *b, Action actions[LEN_ACTIONS], int start_index, int end_index) {
    /*
    駒が成る指手をactionsに追加する.
    start_index <= i < end_index を満たすiについて,
    actions[i].promotion = 1 とした手を追加する.
    歩が成れるときに必ず成ることに注意する.
    返り値 = end_index + 追加した指手の個数
    */
    int old_end_index = end_index;
    for (int i = start_index; i < old_end_index; i++) {
        assert(actions[i].from_stock == 0);
        assert(actions[i].promotion == 0);
        int piece = b->board[actions[i].from_x][actions[i].from_y];
        if (piece <= GIN && (actions[i].to_x == TOP || actions[i].from_x == TOP)) {
            if (piece == FU)
                // 歩が成れるときは必ず成る.
                actions[i].promotion = 1;
            else {
                // 角, 飛, 銀が成る手を追加する.
                actions[end_index] = actions[i];
                actions[end_index++].promotion = 1;
            }
        }
    }
    return end_index;
}


static int get_piece_position(const Board *b, int *x, int *y, int piece) {
    /*
    pieceを1つ見つけて, その位置を*xと*yに代入する.
    pieceがない場合にはxもyも変えない.
    */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (b->board[i][j] == piece) {
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
static int add_drop_actions(const Board *b, Action actions[LEN_ACTIONS], int end_index) {
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

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (b->board[i][j] == EMPTY) {
                int min_k;
                if (i == TOP || j == y)
                    // 最上段や自分の歩がある列には歩を打てない.
                    min_k = 2;
                else
                    min_k = 1;
                for (int k = min_k; k < 6; k++) {
                    if (b->next_stock[k]) {
                        Action action = {k, -1, -1, i, j};
                        actions[end_index++] = action;
                    }
                }
            }
        }
    }
    return end_index;
}


bool is_checking(const Board *b) {
    // 手番側が王手しているかを判定する.

    // 可能な駒移動の指手を列挙する.
    Action all_actions[LEN_ACTIONS];
    int end_index = add_move_actions(b, all_actions, 0);

    // 相手の王の位置を取得する.
    int x, y;
    get_piece_position(b, &x, &y, -OU);

    // 相手の王をとれる状態かを判定する.
    for (int i = 0; i < end_index; i++) {
        if (all_actions[i].to_x == x && all_actions[i].to_y == y)
            return true;
    }
    return false;
}


// reverse_boardを使用
bool is_checked(const Board *b) {
    /*
    手番側が王手されているかを判定する.
    引数のbはconstではなく, 直接編集されることに注意する.
    */
    Board b_copy = *b;
    reverse_board(&b_copy);
    return is_checking(&b_copy);
}


// update_boardを使用
int get_all_actions(const Board *b, Action all_actions[LEN_ACTIONS]) {
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
    for (int i = 0; i < len_tmp_actions; i++) {
        Board next_b = *b;
        update_board(&next_b, tmp_actions[i]);
        if (is_checked(&next_b))
            // 王手放置のとき
            continue;
        if (tmp_actions[i].from_stock == FU &&
            b->board[tmp_actions[i].to_x + move_matrix_x[FU][0]][tmp_actions[i].to_y + move_matrix_y[FU][0]] == -OU) {
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


static int get_number_of_moves(const Board *b) {
    // 手番側の可能な指手の個数を返す.
    Action all_actions[LEN_ACTIONS];
    return get_all_actions(b, all_actions);
}


bool is_checkmate(const Board *b) {
    // 詰みなら1, 詰みでないなら0を返す.
    // 手番側に可能な指手があるかどうかを探す.
    return get_number_of_moves(b) == 0;
}


bool is_possible_action(const Board *b, Action action) {
    // actionが正当なら1, 不当なら0を返す.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions(b, all_actions);
    for (int i = 0; i < len_all_actions; i++) {
        if (action_equal(&action, &all_actions[i]))
            return true;
    }
    return false;
}

bool is_drop_pawn_check(const Board *b, Action action) {
    // actionが歩を打って王手する指手かどうかを判定する.

    int next_x = action.to_x + move_matrix_x[FU][0];
    int next_y = action.to_y + move_matrix_y[FU][0];
    return action.from_stock == FU && b->board[next_x][next_y] == -OU;
}


bool is_useful(const Board *b, const Action *action) {
    /*
    actionが有用かどうかを簡単に判定する.
    飛や角が成れるのに成らない指手のみに対し0を返す.
    */
    if (action->from_stock || action->promotion == 1)
        // 持ち駒を打つときや駒が成るとき
        return true;
    if (b->board[action->from_x][action->from_y] == HISHA || b->board[action->from_x][action->from_y] == KAKU) {
        // 飛や角を動かすとき
        if (action->from_x == TOP || action->to_x == TOP)
            // 駒が成れるとき
            return false;
    }
    // それ以外のとき
    return true;
}


// update_boardとreverse_boardを使用
int get_useful_actions(const Board *b, Action actions[LEN_ACTIONS]) {
    /*
    有用な指手を列挙する.
    相手の王を詰ませられるときは, その1手を代入する.
    そうでないときは, ごく一部の無駄な指手だけを除いて, ほぼ全ての指手を列挙する.
    */

    // 選択可能な指手を全て列挙する.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions(b, all_actions);

    // 相手の王を詰ませられるかを判定する.
    for (int i = 0; i < len_all_actions; i++) {
        Board next_b = *b;
        update_board(&next_b, all_actions[i]);
        reverse_board(&next_b);
        if (is_checkmate(&next_b)) {
            // all_actions[i]を行うと相手の王が詰むとき
            actions[0] = all_actions[i];
            return 1;
        }
    }

    // ほぼ明らかに無駄な指手以外を列挙する.
    int end_index = 0;
    for (int i = 0; i < len_all_actions; i++) {
        if (is_useful(b, &all_actions[i]))
            actions[end_index++] = all_actions[i];
    }

    return end_index;
}


void count_connections(const Board *b, double counts[5][5]) {
    /*
    各マスについて, ききのある駒の数を数える.
    評価関数の入力に用いる.
    */

    // 初期化する.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            counts[i][j] = 0.0;
    }

    // 数える.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = b->board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            if (piece != KAKU && piece != HISHA) {
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する.
                for (int k = 0; k < move_length[piece]; k++) {
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    if (0 <= x && x < 5 && 0 <= y && y < 5) {// && b->board[x][y] <= EMPTY) {
                        counts[x][y] += 1.0;
                    }
                }
            }
            if (piece % NARI == KAKU || piece % NARI == HISHA) {
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する.
                for (int k = 0; k < move_length[piece % NARI]; k++) {
                    int x = i + move_matrix_x[piece % NARI][k];
                    int y = j + move_matrix_y[piece % NARI][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5) {// && b->board[x][y] <= EMPTY) {
                        counts[x][y] += 1.0;
                        // 駒を飛び越えてききがあることにする.
                        /*
                        if (b->board[x][y] < EMPTY)
                            // 相手の駒を取ったとき
                            break;
                        */
                        x += move_matrix_x[piece % NARI][k];
                        y += move_matrix_y[piece % NARI][k];
                    }
                }
            }
        }
    }
}


void piece_moves_to_vector(const Board *b, double vec[], int start_index) {
    /*
    各マスについて, ききのある駒の数を数える.
    評価関数の入力に用いる.
    */

    // 0.0で初期化する.
    for (int i = 0; i < 25*NARI; i++)
        vec[start_index + i] = 0.0;

    // 数える.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = b->board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            if (piece != KAKU && piece != HISHA) {
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する.
                for (int k = 0; k < move_length[piece]; k++) {
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    if (0 <= x && x < 5 && 0 <= y && y < 5) {// && b->board[x][y] <= EMPTY) {
                        vec[start_index + 25*(piece%NARI) + 5*x + y] += 1.0;
                    }
                }
            }
            if (piece % NARI == KAKU || piece % NARI == HISHA) {
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する.
                for (int k = 0; k < move_length[piece % NARI]; k++) {
                    int x = i + move_matrix_x[piece % NARI][k];
                    int y = j + move_matrix_y[piece % NARI][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5) {// && b->board[x][y] <= EMPTY) {
                        vec[start_index + 25*(piece%NARI) + 5*x + y] += 1.0;
                        // 駒を飛び越えてききがあることにする.
                        /*
                        if (b->board[x][y] < EMPTY)
                            // 相手の駒を取ったとき
                            break;
                        */
                        x += move_matrix_x[piece % NARI][k];
                        y += move_matrix_y[piece % NARI][k];
                    }
                }
            }
        }
    }
}


Action delta_of(const Board *before, const Board *after) {
    // 盤面beforeと盤面afterの差は一手分であるとし、beforeは自分が打つ前、
    // afterは自分が打った後まだ回転していない盤面であるとする
    // その差分から行動を求めAction型で返す
    // エラーチェックはなるべくするが、完全ではない

    Action action = {
            .from_stock=EMPTY,
            .from_x=-1,
            .from_y=-1,
            .to_x=-1,
            .to_y=-1,
            .promotion=0
    };
    int gain = EMPTY;  // 相手の駒を取った場合

    // 自分の持ち駒が変化しているか？
    for (int piece = FU; piece <= KIN; ++piece) {
        int delta = before->next_stock[piece] - after->next_stock[piece];
        if (delta == 1) {  // 持ち駒を打った場合
            assert(action.from_stock == EMPTY && gain == EMPTY);
            action.from_stock = piece;
        } else if (delta == -1) {
            assert(action.from_stock == EMPTY && gain == EMPTY);
            gain = piece;
        } else {
            assert(delta == 0);
        }
    }

    if (action.from_stock != EMPTY) {  // 持ち駒を打った場合
        // 盤面を走査
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                if (before->board[i][j] != after->board[i][j]) {
                    assert(action.to_x == -1 && action.to_y == -1);
                    assert(before->board[i][j] == EMPTY);
                    assert(after->board[i][j] == action.from_stock);

                    action.to_x = i;
                    action.to_y = j;
                }
            }
        }

        return action;
    } else {  // 持ち駒を打っていない場合
        int piece_before;  // 動く前の駒
        int piece_after;  // 動いた後の駒

        // 盤面を走査
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                if (before->board[i][j] != after->board[i][j]) {
                    if (after->board[i][j] == EMPTY) {
                        assert(action.from_x == -1 && action.from_y == -1);

                        action.from_x = i;
                        action.from_y = j;
                        piece_before = before->board[i][j];
                    } else {
                        assert(action.to_x == -1 && action.to_y == -1);
                        assert(gain == EMPTY && before->board[i][j] == EMPTY
                               || abs(before->board[i][j]) % NARI == gain);

                        action.to_x = i;
                        action.to_y = j;
                        piece_after = after->board[i][j];
                    }
                }
            }
        }

        assert(abs(piece_before) % NARI == abs(piece_after) % NARI);
        if (abs(piece_before) < abs(piece_after))
            action.promotion = 1;

        return action;
    }
}
