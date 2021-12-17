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
 *   - get_all_actions_with_tfr
 *   - is_possible_action_with_tfr
 *   - is_checkmate_with_tfr
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


int get_all_actions_with_tfr(Board *b, Action all_actions[LEN_ACTIONS], Board history[], int history_len, int turn){
    // 選択可能な指手を全列挙する.
    // get_all_actionsとは異なり, 千日手も考慮して, 反則手を完全に除くものとする.
    // 手番を終えた側がすぐに負けになるような指手は反則手とみなす.
    // すなわち, 連続王手千日手や先手が千日手にもちこむ指手は反則手である.
    // 最後の審判のようなコーナーケースに注意する.

    // 千日手を考慮せずに可能な指手を全列挙する.
    Action tmp_actions[LEN_ACTIONS];
    int len_tmp_actions = get_all_actions(b, tmp_actions);

    // 千日手関連の反則手を削除する.
    int end_index = 0;
    for (int i = 0; i < len_tmp_actions; i++){
        // 連続王手千日手と, 先手が千日手にもちこむ指手を削除する.
        Board next_b = *b;
        update_board(&next_b, tmp_actions[i]);
        int tfr = is_threefold_repetition(history, history_len, &next_b);
        if (tfr == -1 || (tfr == 1 && turn % 2))
            continue;
        // 最後の審判のようなケースを削除する.
        // 先手に千日手を強いるような打ち歩詰めも削除する.
        if (is_drop_pawn_check(b, &tmp_actions[i])){
            reverse_board(&next_b);
            Action next_actions[LEN_ACTIONS];
            if (get_all_actions_with_tfr(&next_b, next_actions, history, history_len+1, turn+1) == 0)
                // 打ち歩詰めのとき
                continue;
        }
        all_actions[end_index++] = tmp_actions[i];
    }

    return end_index;
}

int is_possible_action_with_tfr(Board *b, Action *action, Board history[], int history_len, int turn){
    // 選択可能な指手かどうかを判定する.
    // is_possible_actionとは異なり, 千日手も考慮する.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions_with_tfr(b, all_actions, history, history_len, turn);
    for (int i = 0; i < len_all_actions; i++){
        if (action_equal(action, &all_actions[i]))
            return 1;
    }
    return 0;
}

int is_checkmate_with_tfr(Board *b, Board history[], int history_len, int turn){
    // 詰みかどうかを判定する.
    // is_checkmateとは異なり, 千日手も考慮する.
    // 手番側に選択可能な指手があるときに0, ないときに1を返す.
    Action all_actions[LEN_ACTIONS];
    return get_all_actions_with_tfr(b, all_actions, history, history_len, turn) == 0;
}


#endif  /* BOARD_UTILS */
