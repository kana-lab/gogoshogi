#ifndef GAME_H
#define GAME_H


#include "gamedef.h"
#include "Board.h"
#include "Action.h"


/*********************************
 * Gameクラスの定義
 *********************************/

typedef struct {                  // 試合一回分を表す構造体
    Board current;                // 現在の盤面
    int turn;                     // 現在のターン
    Board history[MAX_TURN * 2];  // 盤面の履歴の配列, turn 個の盤面が入っている
    int history_len;              // 履歴の長さを表す、常にturnと同一の値を取る
} Game;

typedef struct tagPlayerInterface {  // プレイヤーを表すインターフェースクラス
    // get_actionは関数ポインタであり、指手を決める関数を入れておく
    // get_actionはゲームの状況を表すgameと、プレイヤー自身を表すselfを受け取る
    Action (*get_action)(struct tagPlayerInterface *self, const Game *game);
} PlayerInterface;


/*********************************
 * Gameクラスのメソッド
 *********************************/

Game create_game();

int is_threefold_repetition(const Game *game, Action action);

int get_all_actions_with_tfr(const Game *game, Action all_actions[LEN_ACTIONS]);

bool is_possible_action_with_tfr(const Game *game, Action action);

bool is_checkmate_with_tfr(const Game *game);

Action get_previous_action(const Game* game);

int play(Game *game, PlayerInterface *player1, PlayerInterface *player2);


#endif  /* GAME_H */
