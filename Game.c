#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "Game.h"


Game create_game(int max_turn) {
    // Game型の変数を作るコンストラクタ
    // max_turnは保持する履歴の数

    // history配列, is_checking_history配列の動的確保, 確保に失敗したらエラー
    Hash *history = (Hash *) malloc((max_turn + MALLOC_MARGIN) * sizeof(Hash));
    bool *is_checking_history = (bool *) malloc((max_turn + MALLOC_MARGIN) * sizeof(bool));
    assert(history != NULL);
    assert(is_checking_history != NULL);

    // 初期盤面の生成
    Board b = create_board();

    // 履歴に初期盤面を追加
    history[0] = reverse_hash(encode(&b));

    return (Game) {
            .current=b,
            .turn=1,
            .history=history,
            .history_len=1,
            .is_checking_history=is_checking_history,
            .max_turn=max_turn
    };
}


void destruct_game(Game *game) {
    // Game型の変数のデストラクタ
    // 動的確保したメモリの解放を行う

    free(game->history);
    free(game->is_checking_history);
}


Game clone(const Game *game, int max_turn) {
    // gameをコピーする、あとでdestruct_gameで解放必須
    // max_turnは保持する履歴の数を指定する
    // max_turnがgame->max_turnを下回っている場合は、max_turnはgame->max_turnに置き換えられる

    if (max_turn < game->max_turn)
        max_turn = game->max_turn;

    Game game_copy = *game;
    game_copy.max_turn = max_turn;

    // history配列, is_checking_history配列の動的確保, 確保に失敗したらエラー
    Hash *history = (Hash *) malloc((max_turn + MALLOC_MARGIN) * sizeof(Hash));
    bool *is_checking_history = (bool *) malloc((max_turn + MALLOC_MARGIN) * sizeof(bool));
    assert(history != NULL);
    assert(is_checking_history != NULL);

    // history配列, is_checking_history配列のコピー
    for (int i = 0; i < game->history_len; ++i) {
        history[i] = game->history[i];
        is_checking_history[i] = game->is_checking_history[i];
    }

    game_copy.history = history;
    game_copy.is_checking_history = is_checking_history;

    return game_copy;
}


int is_threefold_repetition(const Game *game, Action action) {
    // game->historyには手を打ち終わった直後まだ回転されていない盤面が入っているとする
    // game->currentの盤面にactionを適用したとき、千日手が成立するか否かを判定して返す
    // 通常の千日手の場合は1、連続王手の千日手の場合は-1を返し、千日手でない場合は0を返す
    // 戻り値が-1となった場合は、historyが正常な棋譜であるならば自分の負けである

    int repetition_count = 0;  // 盤面の重複回数
    int first_duplication_index = 0;

    Board b = game->current;
    update_board(&b, action);
    Hash h = encode(&b);

    // 「次に打つプレイヤー」も含めて局面の一致を判定するため、自分の手番のみを見れば良い
    // よって i -= 2 としている
    for (int i = game->history_len - 2; i >= 0; i -= 2) {
        if (hash_equal(h, game->history[i])) {
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
        for (int i = first_duplication_index; i < game->history_len; i += 2) {
            if (!game->is_checking_history[i]) {
                continuous_check = 0;
                break;
            }
        }

        return (continuous_check) ? -1 : 1;
    }

    return 0;
}


int is_threefold_repetition_2(const Game *game) {
    int repetition_count = 0;
    int first_duplication_index = 0;

    Hash h = game->history[game->history_len - 1];
    for (int i = game->history_len - 3; i >= 0; i -= 2) {
        if (hash_equal(h, game->history[i])) {
            ++repetition_count;
            first_duplication_index = i;
        }
    }

    if (repetition_count >= 3) {
        int continuous_check = 1;
        for (int i = first_duplication_index; i < game->history_len; i += 2) {
            if (!game->is_checking_history[i]) {
                continuous_check = 0;
                break;
            }
        }

        return (continuous_check) ? -1 : 1;
    }

    return 0;
}


void do_action(Game *game, Action action) {
    // エラーチェックをせずにactionを実行する
    // デバッグしてない

    update_board(&game->current, action);
    game->is_checking_history[game->history_len] = is_checking(&game->current);
    game->history[game->history_len] = encode(&game->current);
    reverse_board(&game->current);
    ++game->history_len;
    ++game->turn;
}


void undo_action(Game *game) {
    // ゲームを1ターン戻す
    // デバッグしてない

    if (game->history_len < 2)
        return;
    load(game, game->history_len - 1);
}


int save(const Game *game) {
    // gameの現在の状態をセーブしておき、あとで復元するためのIDを返す
    // 未来の状態には戻れない

    return game->history_len;
}


void load(Game *game, int saved_id) {
    // セーブしておいたIDをもとに、状態を復元する

    //assert(game->history_len > saved_id);
    if (game->history_len < saved_id) {
        debug_print("in load: ERROR, history_len=%d, saved_id=%d", game->history_len, saved_id);
        assert(false);
    }

    Hash h = reverse_hash(game->history[saved_id - 1]);
    game->current = decode(h);
    game->history_len = saved_id;
    game->turn = saved_id;
}


int get_all_actions_with_tfr(const Game *game, Action all_actions[LEN_ACTIONS]) {
    // 選択可能な指手を全列挙する.
    // get_all_actionsとは異なり, 千日手も考慮して, 反則手を完全に除くものとする.
    // 手番を終えた側がすぐに負けになるような指手は反則手とみなす.
    // すなわち, 連続王手千日手や先手が千日手にもちこむ指手は反則手である.
    // 最後の審判のようなコーナーケースに注意する.

    // 千日手を考慮せずに可能な指手を全列挙する.
    Action tmp_actions[LEN_ACTIONS];
    int len_tmp_actions = get_all_actions(&game->current, tmp_actions);

    // 千日手関連の反則手を削除する.
    int end_index = 0;
    for (int i = 0; i < len_tmp_actions; i++) {
        // 連続王手千日手と, 先手が千日手にもちこむ指手を削除する.
        int tfr = is_threefold_repetition(game, tmp_actions[i]);
        if (tfr == -1 || (tfr == 1 && game->turn % 2))
            continue;
        // 最後の審判のようなケースを削除する.
        // 先手に千日手を強いるような打ち歩詰めも削除する.
        if (is_drop_pawn_check(&game->current, tmp_actions[i])) {
            do_action((Game *) game, tmp_actions[i]);

            Action next_actions[LEN_ACTIONS];
            if (get_all_actions_with_tfr(game, next_actions) == 0) {
                // 打ち歩詰めのとき
                undo_action((Game *) game);
                continue;
            } else {
                undo_action((Game *) game);
            }
        }
        all_actions[end_index++] = tmp_actions[i];
    }

    return end_index;
}


bool is_possible_action_with_tfr(const Game *game, Action action) {
    // 選択可能な指手かどうかを判定する.
    // is_possible_actionとは異なり, 千日手も考慮する.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions_with_tfr(game, all_actions);
    for (int i = 0; i < len_all_actions; i++) {
        if (action_equal(&action, &all_actions[i]))
            return true;
    }
    return false;
}


bool is_checkmate_with_tfr(const Game *game) {
    // 詰みかどうかを判定する.
    // is_checkmateとは異なり, 千日手も考慮する.
    // 手番側に選択可能な指手があるときに0, ないときに1を返す.
    Action all_actions[LEN_ACTIONS];
    return get_all_actions_with_tfr(game, all_actions) == 0;
}


int get_useful_actions_with_tfr(const Game *game, Action actions[LEN_ACTIONS]) {
    /*
    有用な指手を列挙する.
    相手の王を詰ませられるときは, その1手を代入する.
    そうでないときは, ごく一部の無駄な指手だけを除いて, ほぼ全ての指手を列挙する.
    */

    // 選択可能な指手を全て列挙する.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions_with_tfr(game, all_actions);

    // 相手の王を詰ませられるかを判定する.
    for (int i = 0; i < len_all_actions; i++) {
        do_action((Game *) game, all_actions[i]);
        if (is_checkmate_with_tfr(game)) {
            // all_actions[i]を行うと相手の王が詰むとき
            actions[0] = all_actions[i];
            undo_action((Game *) game);
            return 1;
        } else {
            undo_action((Game *) game);
        }
    }

    // ほぼ明らかに無駄な指手以外を列挙する.
    int end_index = 0;
    for (int i = 0; i < len_all_actions; i++) {
        if (is_useful(&game->current, &all_actions[i]))
            actions[end_index++] = all_actions[i];
    }

    if (end_index == 0) {
        // 全ての指手が削除されたとき, 1手も削除しないことにする.
        // コーナーケース
        for (int i = 0; i < len_all_actions; i++)
            actions[end_index++] = all_actions[i];
    }

    return end_index;
}


int get_perfectly_useful_actions_with_tfr(const Game *game, Action actions[LEN_ACTIONS]) {
    /*
    有用な指手を列挙する.
    相手の王を詰ませられるときは, その1手を代入する.
    ※ただし、上の関数とは違い、後手が通常の千日手を決める形の勝利も詰みと見なしてその一手のみを返す
    ※また、詰みの一手が返されるとき戻り値は-1とする
    そうでないときは, ごく一部の無駄な指手だけを除いて, ほぼ全ての指手を列挙する.
    */

    // 選択可能な指手を全て列挙する.
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions_with_tfr(game, all_actions);

    // 相手の王を詰ませられるかを判定する.
    for (int i = 0; i < len_all_actions; i++) {
        do_action((Game *) game, all_actions[i]);
        int judge_result = judge(game);
        assert(judge_result != 1);
        if (judge_result == -1) {
            // all_actions[i]を行うと相手の王が詰むとき
            actions[0] = all_actions[i];
            undo_action((Game *) game);
            return -1;
        } else {
            undo_action((Game *) game);
        }
    }

    // ほぼ明らかに無駄な指手以外を列挙する.
    int end_index = 0;
    for (int i = 0; i < len_all_actions; i++) {
        if (is_useful(&game->current, &all_actions[i]))
            actions[end_index++] = all_actions[i];
    }

    if (end_index == 0) {
        // 全ての指手が削除されたとき, 1手も削除しないことにする.
        // コーナーケース
        for (int i = 0; i < len_all_actions; i++)
            actions[end_index++] = all_actions[i];
    }

    return end_index;
}


static void print_all_actions_for_debug(Game *game) {
    // 可能な指手を全て出力する

#ifdef DEBUG_MODE
    printf("------------------- all possible actions -------------------\n");

    // 全ての可能な指手をゲットする
    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_all_actions_with_tfr(game, all_actions);

    // 良い感じに整形して表示
    char buf[32];
    for (int i = 0; i < len_all_actions; i++) {
        if (i % 10 == 0 && i)
            puts("");
        action_to_string(all_actions[i], buf);
        printf("%s, ", buf);
    }
    puts("");

    printf("------------------------------------------------------------\n");
#endif
}


Action get_previous_action(const Game *game) {
    // 前のプレイヤーの行動を求めて返す
    // 行動は、前のプレイヤーの座席が盤面の下側であると見た時の行動
    // 1ターン目であり前の行動がない場合にはヌル, すなわち(Action){}を返す

    if (game->history_len < 2)
        return (Action) {};

    Board previous = decode(game->history[game->history_len - 2]);
    reverse_board(&previous);
    Board current = game->current;
    reverse_board(&current);
    return delta_of(&previous, &current);
}


int judge(const Game *game) {
    // 千日手判定+チェックメイト判定を一度に行う
    // 現在のプレイヤーの勝ちであれば1を、負けであれば-1を、いずれでもなければ0を返す

    if (is_checkmate_with_tfr(game))  // 自分の負け
        return -1;

    int tfr = is_threefold_repetition_2(game);
    if (tfr == 1) {  // 相手が通常の千日手を決めたとき
        return (game->turn % 2 == 1) ? -1 : 1;
    } else if (tfr == -1) {  // 相手が連続王手千日手を決めたとき
        return 1;
    }

    return 0;
}


int play(Game *game, PlayerInterface *player1, PlayerInterface *player2, bool debug) {
    // player1を先手、player2を後手としてゲームを行う
    // 先手が勝った場合は1を、後手が勝った場合は-1を返し、引き分けの場合は0を返す
    // AIが指した手の標準出力へのプリントは行われないことに注意

    int winner = 0;
    while (game->turn <= game->max_turn) {  // 150手以内
        // デバッグプリント
        if (debug) {
            Board b = game->current;
            if (game->turn % 2 == 0)
                reverse_board(&b);
            if (game->turn % 2 == 0)
                debug_print("\nTurn %d; Second Player's Turn\n", game->turn);
            else
                debug_print("\nTurn %d; First Player's Turn\n", game->turn);
            print_board_for_debug(&b);
            print_all_actions_for_debug(game);
        }

        int current_player = (game->turn % 2) ? 1 : -1;

        // 次の行動を取ってくる
        Action action;
        if (current_player == 1) {
            action = player1->get_action(player1, game);
        } else {
            assert(current_player == -1);
            action = player2->get_action(player2, game);
        }

        // 取ってきた行動が合法手か？
        if (!is_possible_action_with_tfr(game, action)) {
            debug_print("the specified action is not legal.");
            winner = current_player * (-1);
            break;
        }

        // 実際に駒を動かす
        do_action(game, action);

        // 相手が詰みかどうかをチェック
        if (is_checkmate_with_tfr(game)) {
            if (debug)
                debug_print("checkmate.");
            winner = current_player;
            break;
        }

        // 千日手が成立するか？
        int tfr = is_threefold_repetition_2(game);
        if (tfr) {
            if (tfr == 1 && current_player == -1) {
                // 後手が千日手を決めたとき
                winner = current_player;
            } else if (tfr == -1) {
                // 連続王手千日手が成立したとき
                // エラー処理 (理由: 連続王手千日手は反則手であり, possible_actionではないから)
                debug_print("error: threefold repetition with continuous check");
                winner = current_player * (-1);
            } else {
                // 先手が千日手を決めたとき
                // エラー処理 (理由: 先手が千日手を決める指手は反則手であり, possible_actionではないから)
                debug_print("error: threefold repetition by the first player");
                winner = current_player * (-1);
            }
            break;
        }
    }

    return winner;
}
