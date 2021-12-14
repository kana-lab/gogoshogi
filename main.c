#include "gamedef.c"
#include "gameutils.c"

#define MAX_TURN 150


/*************************
 * 未定義の関数群
 * これから分担して実装する
 *   - get_ai_action
 *************************/

// debug_print関数を積極的に利用し、随所にエラーメッセージを散りばめること！

Action get_user_action(int);  // プロトタイプ宣言

Action get_ai_action(Board *b, int turn) {
    // 盤面bを受け取って、次にAIがどう打つべきかを決定する
    // 次の行動はAction型の変数にして返す
    return get_user_action(turn + 1);
}


/************************
 * 以下プログラムの本流
 ************************/

Action get_user_action(int turn) {
    // ユーザー入力を受けて、その文字列をAction型の変数に翻訳して返す

    // ユーザーからの入力を受ける
    char buf[32] = {};
    scanf("%31s", buf);

    Action action;
    if(0 != string_to_action(buf,&action)){
        debug_print("in get_user_action: invalid input string.");
        abort_game(USER);
    }

    if (turn % 2 == 0)  // 偶数手番のとき盤面の逆転がある
        reverse_action(&action);

    return action;
}

void display_action(Action action, int turn) {
    // actionの表す行動を、"2A3A"等の文字列に変換してプリントする

    if (turn % 2 == 0)  // 偶数手番のとき盤面の逆転がある
        reverse_action(&action);

    char buf[32] = {};
    action_to_string(action, buf);
    puts(buf);
}

void print_all_actions(const Board *b, int turn) {
    // 可能な指手を全て出力する

#ifdef DEBUG_MODE
    printf("------------------- all possible actions -------------------\n");

    // 全ての可能な指手をゲットする
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions(b, all_actions);

    // 良い感じに整形して表示
    char buf[32];
    for (int i = 0; i < len_all_actions; i++) {
        if (i % 10 == 0 && i)
            puts("");
        action_to_string(all_actions[i], buf);
        printf("%s, ", buf);
    }
    if (len_all_actions % 10)
        puts("");

    printf("------------------------------------------------------------\n");
#endif
}

int main(int argc, char *argv[]) {
    // 引数の個数をチェック
    if (argc != 2) {
        puts("the number of command line arguments must be 2.");
        return -1;
    }

    // 先手か後手か
    int first_is_user;
    int second_is_user;
    if (!strcmp(argv[1], "0")) {
        first_is_user = 1;
        second_is_user = 0;
    } else if (!strcmp(argv[1], "1")) {
        first_is_user = 0;
        second_is_user = 1;
    } else {
        puts("invalid command line argument.");
        return -1;
    }

    int first_mover = (first_is_user) ? USER : AI;

    // 初期化済みの盤面を作る
    Board board = create_board(first_mover);

    // 盤面の履歴を保持する配列を宣言し、初期化する
    Board history[MAX_TURN] = {};
    int history_index = 0;
    history[history_index++] = board;

    // ゲームのループをまわし、勝者を決める
    int winner = 0;
    for (int turn = 1; turn <= MAX_TURN; ++turn) {  // 150手以内
        // デバッグプリント
        print_board_for_debug(&board);
        print_all_actions(&board, turn);

        Action action;
        int current_player = (first_is_user + turn) % 2 ? AI : USER;

        // まず、詰みかどうかをチェックする (ステイルメイトは除外)
        if (is_checkmate(&board) && is_checked(board)) {
            debug_print("checkmate.");
            winner = current_player * (-1);
            break;
        }

        // 次の行動を取ってくる (合法手がなくても指さなければならない事に注意)
        if (turn % 2) {  // 奇数手番, 盤面の逆転なし
            if (first_is_user) {
                action = get_user_action(turn);
            } else {
                action = get_ai_action(&board, turn);
                display_action(action, turn);
            }
        } else {  // 偶数手番, 盤面の逆転あり
            if (second_is_user) {
                action = get_user_action(turn);
            } else {
                action = get_ai_action(&board, turn);
                display_action(action, turn);
            }
        }

        // 取ってきた行動が合法手か？ (千日手を除く)
        if (!is_possible_action(&board, &action)) {
            debug_print("the specified action is not legal.");
            winner = current_player * (-1);
            break;
        }

        // 実際に駒を動かす
        update_board(&board, action);

        // 千日手のチェック
        int three_fold_repetition = is_threefold_repetition(history, history_index, &board);
        if (three_fold_repetition == 1) {
            debug_print("three fold repetition has occurred.");
            winner = (first_is_user) ? AI : USER;
            break;
        } else if (three_fold_repetition == -1) {
            debug_print("three fold repetition (with continuous checks) has occurred.");
            winner = current_player * (-1);
            break;
        }

        // 盤面を履歴に追加し、180°回転させる
        history[history_index++] = board;
        reverse_board(&board);
    }

    // 勝敗の表示
    if (winner == USER) {
        puts("You Win");
    } else if (winner == AI) {
        puts("You Lose");
    } else {
        puts("Draw");
    }

    return 0;
}
