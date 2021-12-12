#ifndef BOARD_UTILS
#define BOARD_UTILS

#include "gamedef.c"
#include "possible_actions.c"

//
// このモジュールでは、盤面Boardや指手Actionに関連する関数を定義する
//


/************************************
 * 定義済み関数一覧
 *   - is_threefold_repetition
 ************************************/

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