#ifndef GAME_H
#define GAME_H


#include "gamedef.h"
#include "Board.h"
#include "Action.h"
#include "Hash.h"

#define MALLOC_MARGIN 8  // mallocで動的確保する時にどれだけ余裕を持って確保するか？


/*********************************
 * Gameクラスの定義
 *********************************/

typedef struct {      // 試合一回分を表す構造体
    Board current;    // 現在の盤面
    int turn;         // 現在のターン
    Hash *history;    // 盤面の履歴の配列(要素数はturn個), 配列は動的確保される
    int history_len;  // 履歴の長さを表す、常にturnと同一の値を取る

    // 以下のメンバは内部的なもの(プライベート)であり、使用者が意識する必要はない
    // Game構造体はアドレス渡しが主なので、以下がオーバーヘッドになる事はない
    bool *is_checking_history;  // 各履歴が王手状態にあるか否かを保持する動的配列
    int max_turn;     // この構造体が保持できる履歴の最大数
} Game;

typedef struct tagPlayerInterface {  // プレイヤーを表すインターフェースクラス
    // get_actionは関数ポインタであり、指手を決める関数を入れておく
    // get_actionはゲームの状況を表すgameと、プレイヤー自身を表すselfを受け取る
    Action (*get_action)(struct tagPlayerInterface *self, const Game *game);
} PlayerInterface;


/*********************************
 * Gameクラスのメソッド
 *********************************/

Game create_game(int max_turn);

void destruct_game(Game *game);

Game clone(const Game *game, int max_turn);

void do_action(Game *game, Action action);

void undo_action(Game *game);

int save(const Game *game);

void load(Game *game, int saved_id);

int is_threefold_repetition(const Game *game, Action action);

int is_threefold_repetition_2(const Game *game);

int get_all_actions_with_tfr(const Game *game, Action all_actions[LEN_ACTIONS]);

bool is_possible_action_with_tfr(const Game *game, Action action);

bool is_checkmate_with_tfr(const Game *game);

int get_useful_actions_with_tfr(const Game *game, Action actions[LEN_ACTIONS]);

Action get_previous_action(const Game *game);

int play(Game *game, PlayerInterface *player1, PlayerInterface *player2, bool debug);


#endif  /* GAME_H */
