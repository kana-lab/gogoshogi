#ifndef ACTION_H
#define ACTION_H


#include "gamedef.h"


/*********************************
 * Actionクラスの定義
 *********************************/

typedef struct {     // 駒の移動を表す構造体
    int from_stock;  // 持ち駒を打つ場合その種類、打たない場合0
    int from_x;      // 既存の駒を動かす場合のもとの座標
    int from_y;      // 持ち駒を打つ場合は何でも良い
    int to_x;        // 駒の移動先の座標
    int to_y;
    int promotion;   // 移動後に成るか否か
} Action;


/*********************************
 * Actionクラスのメソッド
 *********************************/

bool action_equal(const Action *action1, const Action *action2);

void reverse_action(Action *action);

void action_to_string(Action action, char return_buffer[32]);

// static method
bool string_to_action(const char *action_string, Action *return_action);


#endif  /* ACTION_H */
