#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "Game.h"


Game create_game(int max_turn) {
    // Game型の変数を作るコンストラクタ
    // max_turnは保持する履歴の数

    // history配列の動的確保, 確保に失敗したらエラー
    Board *history = (Board *) malloc(max_turn * sizeof(Board));
    assert(history != NULL);

    // 初期盤面の生成
    Board b = create_board();

    // 履歴に初期盤面を追加
    Board b_rev = b;
    reverse_board(&b_rev);
    history[0] = b_rev;

    return (Game) {
            .current=b,
            .turn=1,
            .history=history,
            .history_len=1,
            .max_turn=max_turn
    };
}


void destruct_game(Game *game) {
    // Game型の変数のデストラクタ
    // 動的確保したメモリの解放を行う

    free(game->history);
}


Game clone(const Game *game, int max_turn) {
    // gameをコピーする、あとでdestruct_gameで解放必須
    // max_turnは保持する履歴の数を指定する
    // max_turnがgame->max_turnを下回っている場合は、max_turnはgame->max_turnに置き換えられる

    if (max_turn < game->max_turn)
        max_turn = game->max_turn;

    Game game_copy = *game;
    game_copy.max_turn = max_turn;

    // history配列の動的確保, 確保に失敗したらエラー
    Board *history = (Board *) malloc(max_turn * sizeof(Board));
    assert(history != NULL);

    // history配列のコピー
    for (int i = 0; i < game->history_len; ++i)
        history[i] = game->history[i];

    game_copy.history = history;

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

    // 「次に打つプレイヤー」も含めて局面の一致を判定するため、自分の手番のみを見れば良い
    // よって i -= 2 としている
    for (int i = game->history_len - 2; i >= 0; i -= 2) {
        if (board_equal(&b, &game->history[i])) {
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
            if (!is_checking(&game->history[i])) {
                continuous_check = 0;
                break;
            }
        }

        return (continuous_check) ? -1 : 1;
    }

    return 0;
}


static void do_action_without_error_check(Game *game, Action action) {
    // エラーチェックをせずにactionを実行する
    // デバッグしてない

    update_board(&game->current, action);
    game->history[game->history_len++] = game->current;
    reverse_board(&game->current);
    ++game->turn;
}


static void undo_action(Game *game) {
    // ゲームを1ターン戻す
    // デバッグしてない

    if (game->history_len < 2)
        return;
    game->current = game->history[game->history_len - 2];
    reverse_board(&game->current);
    --game->history_len;
    --game->turn;
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
            // ここら辺勝手に書き換えたけど合ってる…？
            do_action_without_error_check((Game *) game, tmp_actions[i]);

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

    Board previous = game->history[game->history_len - 2];
    reverse_board(&previous);
    Board current = game->current;
    reverse_board(&current);
    return delta_of(&previous, &current);
}


int play(Game *game, PlayerInterface *player1, PlayerInterface *player2) {
    // player1を先手、player2を後手としてゲームを行う
    // 先手が勝った場合は1を、後手が勝った場合は-1を返し、引き分けの場合は0を返す
    // AIが指した手の標準出力へのプリントは行われないことに注意

    int winner = 0;
    while (game->turn <= game->max_turn) {  // 150手以内
        // デバッグプリント
        print_board_for_debug(&game->current);
        print_all_actions_for_debug(game);

        int current_player = (game->turn % 2) ? 1 : -1;

        // まず、詰みかどうかをチェックする
        if (is_checkmate_with_tfr(game)) {
            debug_print("checkmate.");
            winner = current_player * (-1);
            break;
        }

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

        // 千日手が成立するか？
        int tfr = is_threefold_repetition(game, action);
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

        // 実際に駒を動かす
        do_action_without_error_check(game, action);
    }

    return winner;
}
