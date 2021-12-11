#ifndef BOARD_UTILS
#define BOARD_UTILS

#include "gamedef.c"

//
// このモジュールでは、盤面Boardや指手Actionに関連する関数を定義する
//


/************************************
 * 定義済み関数一覧
 *   - create_board
 *   - reverse_board
 *   - is_checked
 *   - is_checking
 *   - is_checkmate
 *   - update_board
 *   - reverse_action
 *   - get_all_actions
 *   - get_all_possible_actions
 *   - is_possible_action
 *   - is_threefold_repetition
 ************************************/

Board create_board(int first_mover) {
    // Board型の盤面を作って初期化し、それを戻り値として返す
    // first_mover引数は先手を表し、USER か AI のいずれかである
}

void reverse_board(Board *b) {
    // 盤面を反転させる
    // b.board[5][5]の全要素に-1を掛けて180°回転する
}

int is_checked(Board *b) {
    // 相手の王に対して、王手になっているか否かを判定する
    // 王手であれば1、そうでなければ0を返す
}

int is_checking(Board *b) {
    // 手番側が王手をしているかを判定する
}

int is_checkmate(Board *b) {
    // 盤面bの相手の詰みを判定する (詰みなら1、そうでないなら0を返す)
    // ステイルメイトは詰みの判定にしない事が望ましい
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

        int gain = abs(b->board[action.to_x][action.to_y]);
        if (gain != EMPTY) {  // 移動先に相手の駒がある場合、それを持ち駒に加える
            gain = (gain > NARI) ? gain - NARI : gain;
            ++b->next_stock[gain];
        }

        b->board[action.to_x][action.to_y] = piece;  // 駒を移動先に持っていく
    }
}

void reverse_action(Action *action) {
    // actionの表す動きを、盤面を180°回転させた時の新たな動きに変更する
}


int get_all_actions(Board *b, Action actions[250]) {
    // 盤面bから可能な指手を全列挙し、actions配列に入れる
    // 打ち歩詰めと王手放置は可能なものとする
    // 二歩と行きどころのない駒は判定する？
    // 多分配列の長さは250あれば足りると思いますが、適当に調整して下さい
    // 戻り値はactionsに入れた要素の個数
}

int get_all_possible_actions(Board *b, Action actions[250]) {
    // 盤面bから可能な指手を全列挙し、actions配列に入れる
    // 打ち歩詰め、王手放置、二歩、行きどころのない駒の全てを判定し、
    // 反則手を除いたものを返す
    // ただし連続王手千日手は判定しない
    // 戻り値はactionsに入れた要素の個数
}

int is_possible_action(Board *b, Action action) {
    // actionがget_all_possible_actionに含まれるか否かを返す
    // 含まれるなら1、そうでないなら0を返す
}

int is_threefold_repetition(Board history[], int history_len, Board *b) {
    // 盤面bは自分が手を打ち終わった直後であると見なし、盤面はその後回転していないものとする
    // 同様に、historyには手を打ち終わった直後まだ回転されていない盤面が入っているとする
    // 盤面bは、履歴 history の最後に追加されていないものとする
    // 盤面bに遷移したタイミング、すなわち自分が打ち終わった直後のタイミングにおいて、
    // 千日手が成立したか否かを判定して返す
    // 通常の千日手の場合は1、連続王手の千日手の場合は-1を返し、千日手でない場合は0を返す
    // 戻り値が-1となった場合は、historyが正常な棋譜であるならば自分の負けである

    int repetition_count = 0;  // 盤面の重複回数
    int first_duplication_index = 0;

    // 「次に打つプレイヤー」も含めて局面の一致を判定するため、自分の手番のみを見れば良い
    // よって i -= 2 としている
    for (int i = history_len - 2; i >= 0; i -= 2) {
        if (board_equal(b, &history[i])) {
            ++repetition_count;
            first_duplication_index = i;
        }
    }

    if (repetition_count >= 3) {  // 盤面bとの重複が3回以上あれば千日手
        // 以下、連続王手の判定
        // 結論: 「自分が連続で相手に王手をかけていないか？」という事のみを判定すれば良い
        // ∵) 盤面bは自分が駒を動かした直後であるので、bは王手をかけられている状態ではない
        //     (もしそうなら、自分が王手放置の禁に抵触するので既にはじかれているはずである)
        //     連続王手では、王手をかけられている側が先に千日手になってしまうという事は無い
        //     今、このifブロックの中では自分の千日手が成立しているので、直近で相手は連続王手をしていない
        //     また、自分が打つ直前の状態が自分の王手状態になっている事もない (相手が王手放置の禁に抵触) //

        int continuous_check = 1;
        for (int i = first_duplication_index; i < history_len; i += 2) {
            if (!is_checking(&history[i])) {
                continuous_check = 0;
                break;
            }
        }

        return (continuous_check) ? -1 : 1;
    }

    return 0;
}


#endif  /* BOARD_UTILS */