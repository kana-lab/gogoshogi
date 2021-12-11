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
 *   - is_checkmate
 *   - update_board
 *   - reverse_action
 *   - get_all_actions
 *   - get_all_possible_actions
 *   - is_possible_action
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

int is_checkmate(Board *b) {
    // 盤面bの相手の詰みを判定する (詰みなら1、そうでないなら0を返す)
    // ステイルメイトは詰みの判定にしない事が望ましい
}

void update_board(Board *b, Action action) {
    // 駒を動かして盤面を更新する
    // 一切の反則手のチェックをしない
    // インデックスの範囲のチェックぐらいはするかも
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


#endif  /* BOARD_UTILS */