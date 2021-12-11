#ifndef GAMEDEF
#define GAMEDEF


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define DEBUG_MODE  // デバッグ時以外はコメントアウトすること！


/************************
 * 定数および構造体の定義
 ************************/

#define USER  1   // これらはユーザーとAIを表す定数であり、変更しない事とする
#define AI    -1  // よって、これらの具体値に依存するコードを書いても良い

#define EMPTY 0
#define FU    1
#define KAKU  2
#define HISHA 3
#define GIN   4
#define KIN   5
#define OU    6
#define NARI  6  // 例えば歩が成った場合は FU + NARI と書く

#define MAX_PIECE_NUMBER 10 // 駒を表す数の最大値

typedef struct {            // 盤面を(持ち駒とセットで)入れておく構造体
    int board[5][5];        // 盤面
    int next_stock[6];      // ユーザーの持ち駒
    int previous_stock[6];  // AIの持ち駒
} Board;

typedef struct {     // 駒の移動を表す構造体
    int from_stock;  // 持ち駒を打つ場合その種類、打たない場合0
    int from_x;      // 既存の駒を動かす場合のもとの座標
    int from_y;      // 持ち駒を打つ場合は何でも良い
    int to_x;        // 駒の移動先の座標
    int to_y;
    int promotion;   // 移動後に成るか否か
} Action;

typedef struct {               // 96bitのハッシュを入れる構造体
    unsigned long long lower;  // lowerには下位の64bitを入れる
    unsigned long long upper;  // upperには上位の32bitを入れる
} Hash;


/*****************************
 * ユーティリティー関数の定義
 *   - debug_print
 *   - print_board_for_debug
 *   - abort_game
 *   - encode
 *   - decode
 *   - array_equal
 *   - board_equal
 *   - string_to_action
 *   - action_to_string
 *****************************/

void debug_print(const char *msg, ...) {
    // デバッグプリント用の関数
    // ほぼprintf関数と変わらないが、末尾に改行が入る
    // #define DEBUG_MODE としてDEBUG_MODEを定義すればデバッグプリントが有効になる

#ifdef DEBUG_MODE
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    puts("");
    fflush(stdin);
    va_end(ap);
#endif  /* DEBUG_MODE */
}

void print_board_for_debug(Board *b) {
    // 盤面をプリントするデバッグ用の関数
    // "全"は成銀を、赤字は相手の駒を表す

#ifdef DEBUG_MODE
    static const char *piece_ch[] = {
            "　",
            "歩", "角", "飛", "銀", "金", "王",
            "と", "馬", "龍", "全"
    };

    // 盤面のプリント
    puts("  ＡＢＣＤＥ");
    for (int i = 0; i < 5; ++i) {
        printf("%d ", 5 - i);
        for (int j = 0; j < 5; ++j) {
            int piece = b->board[i][j];

            // ユーザーの駒かAIの駒かによって色を変える
            if (piece < 0) {
                printf("\x1b[31m");
            } else {
                printf("\x1b[39m");
            }

            printf("%s", piece_ch[abs(piece)]);
        }
        printf("\x1b[39m\n");
    }

    // ユーザーの持ち駒のプリント
    puts("");
    printf("me      : ");
    for (int piece = 0; piece < 6; ++piece) {
        int count = b->next_stock[piece];
        for (int i = 0; i < count; ++i)
            printf("%s", piece_ch[piece]);
    }

    // AIの持ち駒のプリント
    puts("");
    printf("opponent: ");
    for (int piece = 0; piece < 6; ++piece) {
        int count = b->previous_stock[piece];
        for (int i = 0; i < count; ++i)
            printf("%s", piece_ch[piece]);
    }

    printf("\n\n");
#endif  /* DEBUG_MODE */
}

void abort_game(int loser) {
    // ゲームを強制終了する
    // loser引数には USER または AI を指定し、これは敗者を表す
    // この関数はエラー時の強制終了に用いることが望ましい

    debug_print("abort called.");

    if (loser == AI) {
        puts("You Win");
    } else if (loser == USER) {
        puts("You Lose");
    } else {
        debug_print("unknown value of the argument loser: %d", loser);
    }

    exit(1);
}

typedef union {                       // ハッシュのエンコード及びデコード時に使う共用体
    unsigned char to_char;            // ハッシュを整数値として保持
    struct {
        unsigned char index: 5;       // 駒の位置 (持ち駒の場合は25とする)
        unsigned char owner: 1;       // 所有 (0なら自分、1なら相手)
        unsigned char promoted: 1;    // 成っているか否か
        unsigned char is_enabled: 1;  // 余った1ビットは、このハッシュの領域が有効か否かを表すのに使う
    };
} __attribute__((__packed__)) HashField;  // 構造体のザイズがジャスト1byteであることを保証

Hash encode(Board *b) {
    // 盤面bを96bitのハッシュに潰す
    // 駒は2枚×6種であり、各種別ごとに2byteのフィールドを与える (計96bit)
    // 2byteは値の小さい順にソートし、一意性を確保する

    HashField field[8][2] = {};  // Hashへのキャストの都合上インデックスを6ではなく8としている

    // 盤面を探索し、fieldに駒の情報を詰める
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            int piece = b->board[i][j];

            if (piece != EMPTY) {
                int kind_of_piece = abs(piece);
                if (kind_of_piece > NARI)
                    kind_of_piece -= NARI;

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

int hash_equal(Hash h1, Hash h2) {
    // 2つのハッシュ値が等しいか否かを判定
    // 等しければ1、等しくなければ0を返す

    return (h1.lower == h2.lower) && (h1.upper == h2.upper);
}

int board_equal(Board *b1, Board *b2) {
    // 2つの盤面が等しければ1を、等しくなければ0を返す
    // 次に打つプレイヤーが誰かは考慮しない

    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            if (b1->board[i][j] != b2->board[i][j])
                return 0;

    for (int i = 0; i < 6; ++i) {
        if (b1->next_stock[i] != b2->next_stock[i])
            return 0;
        if (b1->previous_stock[i] != b2->previous_stock[i])
            return 0;
    }

    return 1;
}

int action_equal(Action *action1, Action *action2) {
    // action1とaction2の同一性を判定
    // 感覚的には return action1 == action2
    if (action1->from_stock != action2->from_stock || action1->to_x != action2->to_x || action1->to_y != action2->to_y)
        return 0;
    else if (action1->from_stock)
        return 1;
    else
        return action1->from_x == action2->from_x && action1->from_y == action2->from_y &&
               action1->promotion == action2->promotion;
}

Action string_to_action(const char *action_string) {
    // action_stringは「グループ課題: 2回目」のページで指定されている、駒の動きを表す文字列
    // これを解析し、Action型の変数に詰め込んで戻り値として返す
    // 例えば"3CGI"が入力された場合、aをAction型の変数として
    //   a.from_stock = GI, a.to_x = 2, a.to_y = 2, a.turn_over = 0
    // となる。なお、この場合 a.from_x, a.from_y は何でも良い。
}

void action_to_string(Action action, char return_buffer[32]) {
    // actionの表す駒の動きを、「グループ課題: 2回目」のページで指定されているフォーマットに
    // 従った文字列に翻訳し、return_bufferに入れる
    // 文字列の最後には番兵として(数字の)0を入れること！
}


#endif  /* GAMEDEF */
