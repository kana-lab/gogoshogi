#ifndef GAMEDEF_H
#define GAMEDEF_H


#include <time.h>


/************************
 * 定数の定義
 ************************/

//#define DEBUG_MODE  // デバッグ時以外はコメントアウトすること！

#define EMPTY 0
#define FU    1
#define KAKU  2
#define HISHA 3
#define GIN   4
#define KIN   5
#define OU    6
#define NARI  6  // 例えば歩が成った場合は FU + NARI と書く

#define MAX_PIECE_NUMBER 10   // 駒を表す数の最大値
#define TOP              0    // 駒が成れるx座標
#define LEN_ACTIONS      250  // 選択可能な指手の個数の最大値
#define MAX_TURN         150  // ターンの最大数
#define MAX_TIME         9.9  // 1手あたりの最大思考時間(s)


/************************
 * デバッグプリント用の関数
 ************************/

void debug_print(const char *msg, ...);

double stop_watch(struct timespec start_time, struct timespec end_time);

#endif  /* GAMEDEF_H */
