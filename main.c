#include "gamedef.c"
#include "gameutils.c"

#define MAX_TURN 150


/*************************
 * 未定義の関数群
 * これから分担して実装する
 *   - is_threefold_repetition
 *   - stop_watch
 *   - move_piece
 *   - get_ai_action
 *************************/

// debug_print関数を積極的に利用し、随所にエラーメッセージを散りばめること！

void is_threefold_repetition(Board *b, Action action, int first_mover) {
    // 千日手の判定をし、千日手の場合は後手勝ちにする
    // ただし、後手が連続王手千日手をした場合は先手勝ちにする
}

void stop_watch(void (*func)()) {
    // AIの思考時間が9.9秒以内であることを確認する
    // とりあえず引数は関数ポインタにしておきましたが、自由に変更して下さい
}


void move_piece(Board *b, Action action) {
    // actionで示される行動をもとに、盤面bを変更する
    // actionは b->next_player で表されるプレイヤーの行動である
    // この関数内で、種々のエラーチェックを行うが、エラー毎にdebug_printを徹底すること！
    // 恐らくこの関数を書くのは大変 (課題のサイトの「反則手をさした場合」の項を参照)
    // さらに細分化する余地がありそう
}

Action get_ai_action(Board *b, int turn) {
    // 盤面bを受け取って、次にAIがどう打つべきかを決定する
    // 次の行動はAction型の変数にして返す
}


/************************
 * 以下プログラムの本流
 ************************/

Action get_user_action(int turn) {
    // ユーザー入力を受けて、その文字列をAction型の変数に翻訳して返す

    // ユーザーからの入力を受ける
    char buf[32] = {};
    scanf("%31s", buf);

    Action action = string_to_action(buf);

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

    // 初期化済みの盤面を作る
    Board board = create_board(first_is_user ? USER : AI);
    print_board_for_debug(&board);

    // ゲームのループをまわし、勝者を決める
    int winner = 0;
    for (int turn = 1; turn <= MAX_TURN; ++turn) {  // 150手以内
        Action action;

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

        move_piece(&board, action);
        print_board_for_debug(&board);

        if (is_checkmate(&board)) {  // 勝敗が付いているかのチェック
            winner = (first_is_user + turn) % 2 ? USER : AI;
            break;
        }

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