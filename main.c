#include "gamedef.c"


/*************************
 * 未定義の関数群
 * これから分担して実装する
 *   - create_board
 *   - reverse_board
 *   - reverse_action
 *   - string_to_action
 *   - action_to_string
 *   - encode
 *   - decode
 *   - array_equal
 *   - get_all_actions
 *   - is_checked
 *   - get_all_possible_actions
 *   - is_checkmate
 *   - is_possible_action
 *   - update_board
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

void reverse_board(Board *b) {
    // 盤面を反転させる
    // b.board[5][5]の全要素に-1を掛けて180°回転する
}

void reverse_action(const char *action_string, char return_buffer[32]) {
    // action_stringは「グループ課題: 2回目」のページで指定されている、駒の動きを表す文字列
    // 盤面を180°回転させた時の新たな動きの文字列を、return_bufferに入れる
    // 文字列の最後には番兵として(数字の)0を入れること！
}

Action string_to_action(const char *action_string) {
    // action_stringは「グループ課題: 2回目」のページで指定されている、駒の動きを表す文字列
    // これを解析し、Action型の変数に詰め込んで戻り値として返す
}

void action_to_string(Action action, char return_buffer[32]) {
    // actionの表す駒の動きを、「グループ課題: 2回目」のページで指定されているフォーマットに
    // 従った文字列に翻訳し、return_bufferに入れる
    // 文字列の最後には番兵として(数字の)0を入れること！
}

Hash encode(Board *b) {
    // 盤面bを96bitのハッシュに潰す
}

Board decode(Hash h) {
    // ハッシュ値hをBoardに展開する
}

int array_equal(Hash h1, Hash h2) {
    // 2つのハッシュ値が等しいか否かを判定
    // 等しければ1、等しくなければ0を返す
}

int get_all_actions(Board *b, Action actions[250]) {
    // 盤面bから可能な指手を全列挙し、actions配列に入れる
    // 打ち歩詰めと王手放置は可能なものとする
    // 二歩と行きどころのない駒は判定する？
    // 多分配列の長さは250あれば足りると思いますが、適当に調整して下さい
    // 戻り値はactionsに入れた要素の個数
}

int is_checked(Board *b) {
    // 相手の王に対して、王手になっているか否かを判定する
    // 王手であれば1、そうでなければ0を返す
}

int get_all_possible_actions(Board *b, Action actions[250]) {
    // 盤面bから可能な指手を全列挙し、actions配列に入れる
    // 打ち歩詰め、王手放置、二歩、行きどころのない駒の全てを判定し、
    // 反則手を除いたものを返す
    // ただし連続王手千日手は判定しない
    // 戻り値はactionsに入れた要素の個数
}

int is_checkmate(Board *b) {
    // 盤面bの相手の詰みを判定する (詰みなら1、そうでないなら0を返す)
    // ステイルメイトは詰みの判定にしない事が望ましい
}

int is_possible_action(Board *b, Action action) {
    // actionがget_all_possible_actionに含まれるか否かを返す
    // 含まれるなら1、そうでないなら0を返す
}

void update_board(Board *b, Action action) {
    // 駒を動かして盤面を更新する
    // 一切の反則手のチェックをしない
    // インデックスの範囲のチェックぐらいはするかも
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