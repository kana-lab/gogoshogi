#ifndef BOARD_H
#define BOARD_H


#include <stdbool.h>
#include "gamedef.h"
#include "Action.h"


/*********************************
 * Boardクラスの定義
 *********************************/

typedef struct {            // 盤面を(持ち駒とセットで)入れておく構造体
    int board[5][5];        // 盤面
    int next_stock[6];      // 手番側の持ち駒
    int previous_stock[6];  // 手番ではない方の持ち駒
} Board;

typedef struct {               // 96bitのハッシュを入れる構造体
    unsigned long long lower;  // lowerには下位の64bitを入れる
    unsigned long long upper;  // upperには上位の32bitを入れる
} Hash;


/*********************************
 * Boardクラスのメソッド
 *********************************/

Board create_board();

void print_board_for_debug(const Board *b);

bool board_equal(const Board *b1, const Board *b2);

void reverse_board(Board *b);

void update_board(Board *b, Action action);

bool is_checking(const Board *b);

bool is_checked(const Board *b);

bool is_checkmate(const Board *b);

bool is_possible_action(const Board *b, Action action);

bool is_drop_pawn_check(const Board *b, Action action);

int get_all_actions(const Board *b, Action all_actions[LEN_ACTIONS]);

int get_useful_actions(const Board *b, Action actions[LEN_ACTIONS]);

Action delta_of(const Board *before, const Board *after);

Hash encode(const Board *b);

// static method
Board decode(Hash h);


#endif  /* BOARD_H */