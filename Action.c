#include <stdbool.h>
#include "Action.h"


bool action_equal(const Action *action1, const Action *action2) {
    if (action1->from_stock != action2->from_stock
        || action1->to_x != action2->to_x
        || action1->to_y != action2->to_y)
        return false;
    else if (action1->from_stock)
        return true;
    else
        return (action1->from_x == action2->from_x
                && action1->from_y == action2->from_y
                && action1->promotion == action2->promotion);
}


void reverse_action(Action *action) {
    // actionの表す動きを、盤面を180°回転させた時の新たな動きに変更する

    action->from_x = 4 - action->from_x;
    action->from_y = 4 - action->from_y;
    action->to_x = 4 - action->to_x;
    action->to_y = 4 - action->to_y;
}


void action_to_string(Action action, char return_buffer[32]) {
    // actionの表す駒の動きを、「グループ課題: 2回目」のページで指定されているフォーマットに
    // 従った文字列に翻訳し、return_bufferに入れる
    // 文字列の最後には番兵として(数字の)0を入れること！

    if (action.from_stock) {
        return_buffer[0] = '5' - action.to_x;
        return_buffer[1] = 'A' + action.to_y;

        if (action.from_stock == FU) {
            return_buffer[2] = 'F';
            return_buffer[3] = 'U';
        } else if (action.from_stock == KAKU) {
            return_buffer[2] = 'K';
            return_buffer[3] = 'K';
        } else if (action.from_stock == HISHA) {
            return_buffer[2] = 'H';
            return_buffer[3] = 'I';
        } else if (action.from_stock == GIN) {
            return_buffer[2] = 'G';
            return_buffer[3] = 'I';
        } else if (action.from_stock == KIN) {
            return_buffer[2] = 'K';
            return_buffer[3] = 'I';
        }

        return_buffer[4] = '\0';

    } else {
        return_buffer[0] = '5' - action.from_x;
        return_buffer[1] = 'A' + action.from_y;
        return_buffer[2] = '5' - action.to_x;
        return_buffer[3] = 'A' + action.to_y;
        if (action.promotion) {
            return_buffer[4] = 'N';
            return_buffer[5] = '\0';
        } else {
            return_buffer[4] = '\0';
        }
    }
}


bool string_to_action(const char *action_string, Action *return_action) {
    // action_stringは「グループ課題: 2回目」のページで指定されている、駒の動きを表す文字列
    // これを解析し、return_actionに詰め込む
    // action_string が不正な文字列であった時はfalseを、正しい文字列であったときはtrueを返す
    // 例えば"3CGI"が入力された場合、aをAction型の変数として
    //   a.from_stock = GI, a.to_x = 2, a.to_y = 2, a.turn_over = 0
    // となる。なお、この場合 a.from_x, a.from_y は何でも良い。

    static const char *piece_ch[] = {
            "", "FU", "KK", "HI", "GI", "KI"
    };

    Action action = {.from_stock=EMPTY, .promotion=0};  // 解析結果を入れる変数
    const char *str = action_string;  // 名前が長いのでエイリアスにする
    int ptr = 0;

    // action_string の1文字目の解析
    if (str[ptr] < '1' || '5' < str[ptr]) return false;
    action.from_x = action.to_x = '5' - str[ptr++];

    // action_string の2文字目の解析
    if (str[ptr] < 'A' || 'E' < str[ptr]) return false;
    action.from_y = action.to_y = str[ptr++] - 'A';

    // action_string の3文字目は数字かそれ以外かで分岐
    if ('1' <= str[ptr] && str[ptr] <= '5') {  // 3文字目が数字のとき
        action.to_x = '5' - str[ptr++];

        // action_string の4文字目の解析
        if (str[ptr] < 'A' || 'E' < str[ptr]) return false;
        action.to_y = str[ptr++] - 'A';

        // action_string の5文字目に 'N' があるなら成る
        if (str[ptr] == 'N') {
            action.promotion = 1;
            ptr++;
        }
    } else {  // 3文字目が数字でないとき
        for (int p = FU; p <= KIN; ++p) {  // piece_ch の要素のどれかにマッチするまでループ
            if (str[ptr] == piece_ch[p][0] && str[ptr + 1] == piece_ch[p][1]) {
                action.from_stock = p;
                goto MATCHED;
            }
        }
        return false;  // piece_ch のどの要素にもマッチしなかった

        MATCHED:  // piece_ch のどれかにマッチした
        ptr += 2;
    }

    // action_string の最後に余計な文字がないことを確認
    if (str[ptr] != '\0')
        return false;

    *return_action = action;
    return true;
}
