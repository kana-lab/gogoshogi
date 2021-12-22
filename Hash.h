#ifndef HASH_H
#define HASH_H


#include <stdbool.h>
#include "Board.h"


/*********************************
 * Hashクラスの定義
 *********************************/

typedef struct {               // 96bitのハッシュを入れる構造体
    unsigned long long lower;  // lowerには下位の64bitを入れる
    unsigned long long upper;  // upperには上位の32bitを入れる
} Hash;


/*********************************
 * Hashクラスのメソッド
 *********************************/

Hash encode(const Board *b);

Board decode(Hash h);

Hash reverse_hash(Hash h);

bool hash_equal(Hash h1, Hash h2);


#endif  /* HASH_H */