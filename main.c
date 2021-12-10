#include "gamedef.c"


/*************************
 * 未定義の関数群
 * これから分担して実装する
 *   - create_board
 *   - is_threefold_repetition
 *   - stop_watch
 *   - get_user_action
 *   - move_piece
 *   - get_ai_action
 *   - display_action
 *   - is_game_over
 *************************/

// debug_print関数を積極的に利用し、随所にエラーメッセージを散りばめること！

Board create_board(int first_mover) {
    // Board型の盤面を作って初期化し、それを戻り値として返す
    // first_mover引数は先手を表し、USER か AI のいずれかである
}

void is_threefold_repetition(Board *b, Action action, int first_mover) {
    // 千日手の判定をし、千日手の場合は後手勝ちにする
    // ただし、後手が連続王手千日手をした場合は先手勝ちにする
}

void stop_watch(void (*func)()) {
    // AIの思考時間が9.9秒以内であることを確認する
    // とりあえず引数は関数ポインタにしておきましたが、自由に変更して下さい
}

Action get_user_action() {
    // ユーザー入力を受けて、その文字列をAction型の変数に翻訳して返す
    // 例えば"3CGI"が入力された場合、aをAction型の変数として
    //   a.from_stock = GI, a.to_x = 2, a.to_y = 2, a.turn_over = 0
    // となる。なお、この場合 a.from_x, a.from_y は何でも良い。
}

void move_piece(Board *b, Action action) {
    // actionで示される行動をもとに、盤面bを変更する
    // actionは b->next_player で表されるプレイヤーの行動である
    // この関数内で、種々のエラーチェックを行うが、エラー毎にdebug_printを徹底すること！
    // 恐らくこの関数を書くのは大変 (課題のサイトの「反則手をさした場合」の項を参照)
    // さらに細分化する余地がありそう
}

Action get_ai_action(Board *b) {
    // 盤面bを受け取って、次にAIがどう打つべきかを決定する
    // 次の行動はAction型の変数にして返す
}

void display_action(Action action) {
    // actionの表す行動を、"2A3A"等の文字列に変換してプリントする
}

int is_game_over(Board *b) {
    // 盤面bを受け取り、 b->next_player で表される次のプレイヤーが詰まされているか否かを判断する
    // 戻り値として勝者を表す整数 (AI または USER) を返し、勝敗が付いていなければ0を返す
    // 例えば、 b->next_player = USER であったとし、ユーザーが詰みであるならば、AI を返す
    // 王手がかかっていなくても詰みになる状態がある (「ステイルメイト」) が、
    // ステイルメイトの場合は次のターンに自動的に相手が反則負けになるので、この関数では
    // 王手がかかっている状況のみを考えればよい
}


/***********************************
 * 以下プログラムの本流であるmain関数
 ***********************************/

int main(int argc, char *argv[]) {
    // 引数の個数をチェック
    if (argc != 2) {
        puts("the number of command line arguments must be 2.");
        return -1;
    }

    // 先手か後手か
    int first_mover;
    if (!strcmp(argv[1], "0")) {
        first_mover = USER;
    } else if (!strcmp(argv[1], "1")) {
        first_mover = AI;
    } else {
        puts("invalid command line argument.");
        return -1;
    }

    // 初期化済みの盤面を作る
    Board board = create_board(first_mover);
    print_board_for_debug(&board);

    // ゲームのループをまわし、勝者を決める
    int winner;
    for (int i = 0; i < 150; ++i) {  // 150手以内
        Action action;

        if (board.next_player == USER) {
            action = get_user_action();
        } else if (board.next_player == AI) {
            action = get_ai_action(&board);
            display_action(action);
        } else {
            debug_print("ERROR: unknown player %d", board.next_player);
            exit(1);
        }

        move_piece(&board, action);
        print_board_for_debug(&board);

        if ((winner = is_game_over(&board)))  // 勝敗が付いているかのチェック
            break;
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