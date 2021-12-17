#ifndef MONTECARLO
#define MONTECARLO

//
// このモジュールではモンテカルロ木探索に関する機能を実装する
// 参考:
//   wikipedia: https://ja.wikipedia.org/wiki/モンテカルロ木探索
//   Platinum Data Blog: https://blog.brainpad.co.jp/entry/2018/04/05/163000
//

#include "gamedef.c"


#define EXPANSION_THRESHOLD 10
#define

typedef struct tagNode {     // モンテカルロ法で探索する木の頂点を表す構造体
    Board b;                 // 盤面
    int is_checkmate;        // 何回も勝敗を判定するのは無駄なのでメモしておく
    int visit_count;         // この頂点が訪問された回数
    int win_count;           // このノード以下の葉ノードで自分が勝ちとなるものの個数
    struct tagNode child[];  // 子ノードの配列
    int child_len;           // 子ノードの配列の長さ
} Node;


Node *select(Node *root) {
    // モンテカルロ法におけるselectionを行う
    // rootの子ノードの中からQ値+コストが最大になるものを選び、そのアドレスを返す (UCTの場合)
    // もし子ノードがなければNULLを返す
    // もしQ値+コストが無限大となるものがあれば、それを返す
}

void expand(Node *root) {
    // rootに新たな子ノードを1つ加える (本当に1つ？文献によって違う)
    // 1つの場合の疑問点: 選び方はランダム？ それ以上新たに加えられる指手がない場合はスルー？
    // 1つではなく全ての場合はrootは葉ノードである必要がある
    // 全ての場合の疑問点: expansion後、各ノードのカウントはリセット？それともそのまま？
    // とりあえず、expansionは考えられる全ての場合を展開することにし、カウンタはリセットしないとする
    // なお、加えるNode構造体はcalloc関数により動的に確保する
}

int evaluate(Board b) {
    // 盤面bからランダムにプレイアウトを行い勝敗を判定する
    // 自分が勝った場合1を、相手が勝った場合0を返す
    // 千日手どうしよう
}

Node* play(Node* root){
    // rootの子ノードの中から次に打つべき手を選んで、そのアドレスを返す
}

int monte_carlo_tree_search(Node *root) {
    // モンテカルロ木探索を1回行い、勝った場合1を、負けた場合0を返す

    ++root->visit_count;
    int i_win;

    Node *child = select(root);
    if (child == NULL) {  // rootが葉ノードであった場合
        if (root->is_checkmate) {
            i_win = 0;
        } else if (root->visit_count >= EXPANSION_THRESHOLD) {
            expand(root);
            --root->visit_count;
            i_win = monte_carlo_tree_search(root);
        } else {
            i_win = evaluate(root->b);
        }
    } else {  // rootが内部ノードであった場合
        int opponent_win = monte_carlo_tree_search(child);
        i_win = (opponent_win) ? 0 : 1;
    }

    root->win_count += i_win;
    return i_win;
}

Action delta_of(const Board* before, const Board *after){
    // beforeとafterの差は一手分であるとし、その一手を検出して戻り値として返す
}

Action get_ai_action_sample(Board history[], int history_len, Board* b, int turn, Node* cache){
    // 制限時間内でmonte_carlo_tree_searchを繰り返し行い、play関数で最善手を得て返す
}


#endif  /* MONTECARLO */