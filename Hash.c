#include <stdlib.h>
#include <assert.h>
#include "Hash.h"


typedef union {                       // ハッシュのエンコード及びデコード時に使う共用体
    unsigned char to_char;            // ハッシュを整数値として保持
    struct {
        unsigned char index: 5;       // 駒の位置 (持ち駒の場合は25とする)
        unsigned char owner: 1;       // 所有 (0なら自分、1なら相手)
        unsigned char promoted: 1;    // 成っているか否か
        unsigned char is_enabled: 1;  // 余った1ビットは、このハッシュの領域が有効か否かを表すのに使う
    };
} __attribute__((__packed__)) HashField;  // 構造体のザイズがジャスト1byteであることを保証


Hash encode(const Board *b) {
    // 盤面bを96bitのハッシュに潰す
    // 駒は2枚×6種であり、各種別ごとに2byteのフィールドを与える (計96bit)
    // 2byteは値の小さい順にソートし、一意性を確保する

    HashField field[8][2] = {};  // Hashへのキャストの都合上インデックスを6ではなく8としている

    // 盤面を探索し、fieldに駒の情報を詰める
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            int piece = b->board[i][j];

            if (piece != EMPTY) {
                int kind_of_piece = (abs(piece) != OU) ? abs(piece) % NARI : OU;
                HashField *hf = &field[kind_of_piece - 1][0];

                if (hf->is_enabled)
                    ++hf;

                if (hf->is_enabled)
                    debug_print("in encode: three same kind of piece does exist.");

                hf->index = i * 5 + j;
                hf->owner = (piece > 0) ? 0 : 1;
                hf->promoted = (abs(piece) > NARI) ? 1 : 0;
                hf->is_enabled = 1;
            }
        }
    }

    // 自分の持ち駒を探索し、fieldに駒の情報を詰める
    for (int piece = FU; piece <= KIN; ++piece) {
        int number_of_piece = b->next_stock[piece];

        for (int i = 0; i < number_of_piece; ++i) {
            HashField *hf = &field[piece - 1][0];
            if (hf->is_enabled)
                ++hf;

            if (hf->is_enabled)
                debug_print("in encode: three same kind of piece does exist.");

            hf->index = 25;
            hf->owner = 0;
            hf->promoted = 0;
            hf->is_enabled = 1;
        }
    }

    // 相手の持ち駒を探索し、fieldに駒の情報を詰める
    for (int piece = FU; piece <= KIN; ++piece) {
        int number_of_piece = b->previous_stock[piece];

        for (int i = 0; i < number_of_piece; ++i) {
            HashField *hf = &field[piece - 1][0];
            if (hf->is_enabled)
                ++hf;

            if (hf->is_enabled)
                debug_print("in encode: three same kind of piece does exist.");

            hf->index = 25;
            hf->owner = 1;
            hf->promoted = 0;
            hf->is_enabled = 1;
        }
    }

    // 各駒に割り当てられた2byteをソートする
    for (int i = 0; i < 6; ++i) {
        if (!field[i][0].is_enabled || !field[i][1].is_enabled)
            debug_print("in encode: the number of a certain kind of pieces is less than 2.");

        if (field[i][0].to_char > field[i][1].to_char) {
            HashField memo = field[i][0];
            field[i][0] = field[i][1];
            field[i][1] = memo;
        }
    }

    // コンパイラによるアラインによって Hash <-> HashField[16] 間のキャストが
    // 失敗する可能性があるため、static_assertでコンパイル時にエラーチェックする
    static_assert(sizeof(field) == 16, "sizeof(field) != 16");
    static_assert(sizeof(Hash) == 16, "sizeof(Hash) != 16");

    return *((Hash *) field);
}


Board decode(Hash h) {
    // ハッシュ値hをBoardに展開する

    Board b = {};
    HashField (*field)[2] = (HashField (*)[2]) &h;  // ハッシュhを HashField の配列に展開

    for (int piece = FU; piece <= OU; ++piece) {  // 各駒のハッシュを元に盤面bを埋める
        for (int i = 0; i < 2; ++i) {
            HashField hash = field[piece - 1][i];

            if (hash.is_enabled == 0)
                debug_print("in decode the number of a certain kind of pieces is less than 2.");

            int index = hash.index;
            if (index < 25) {  // 駒が盤上にある場合
                // 駒pieceに成りの情報と所有の情報を付け加える
                int piece_with_owner = (hash.promoted) ? piece + NARI : piece;
                piece_with_owner *= (hash.owner) ? -1 : 1;

                // 盤面に配置
                b.board[index / 5][index % 5] = piece_with_owner;
            } else if (index == 25) {  // 駒が持ち駒である場合
                if (hash.promoted)
                    debug_print("in decode: promoted flag of a piece in stock is True.");

                if (hash.owner == 0) {  // 自分の駒である場合
                    ++b.next_stock[piece];
                } else {  // 相手の駒である場合
                    ++b.previous_stock[piece];
                }
            }
        }
    }

    return b;
}


Hash reverse_hash(Hash h) {
    HashField (*field)[2] = (HashField (*)[2]) &h;

    for (int piece = FU; piece <= OU; ++piece) {
        for (int i = 0; i < 2; ++i) {
            HashField *hf = &field[piece - 1][i];
            hf->owner = 1 - hf->owner;
            if (hf->index != 25)
                hf->index = 24 - hf->index;
        }
    }

    for (int i = 0; i < 6; ++i) {
        if (field[i][0].to_char > field[i][1].to_char) {
            HashField memo = field[i][0];
            field[i][0] = field[i][1];
            field[i][1] = memo;
        }
    }

    return *((Hash *) field);
}


bool hash_equal(Hash h1, Hash h2) {
    return (h1.lower == h2.lower) && (h1.upper == h2.upper);
}