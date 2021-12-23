#ifndef MONTECARLO
#define MONTECARLO

//
// このモジュールではモンテカルロ木探索に関する機能を実装する
// 参考:
//   wikipedia: https://ja.wikipedia.org/wiki/モンテカルロ木探索
//   Platinum Data Blog: https://blog.brainpad.co.jp/entry/2018/04/05/163000
//

//
// デバッグはほとんどしてないですが、動くことは確認しています
// MonteCarloAI同士の自己対局を見た感じ、非常に弱いです
//


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "Action.h"
#include "Board.h"
#include "Hash.h"
#include "Game.h"
#include "gamedef.h"
#include "neural_network/random_move.c"

#define EXPANSION_THRESHOLD 50
#define MAX_SIMULATION_TURN 500


typedef struct tagNode {
    Action action;  // このノードに遷移する直前の行動を入れる
    // ※is_checkmateは暫定的に自分の負けなら1、そうでないなら0
    int is_checkmate;  // 自分の勝ちなら1, 相手の勝ちなら-1, 引き分けなら0
    int visit_count;
    int win_count;
    struct tagNode *children;
    int child_len;
} Node;


Node create_node(Action action, const Game *game) {
    // gameは現在の盤面, actionはそこから取り得る行動
    // Node型変数を新たに作成するコンストラクタであり、is_checkmateメンバも埋める

    Node node;
    if (game == NULL) {
        node = (Node) {
                .action=action,
                .is_checkmate=0,
                .visit_count=0,
                .win_count=0,
                .children=NULL,
                .child_len=0
        };
    } else {
        assert(is_possible_action_with_tfr(game, action));

        // actionの表す行動を取った時チェックメイトになるか？
        do_action((Game *) game, action);
        int is_checkmate = is_checkmate_with_tfr(game);
        undo_action((Game *) game);

        // 動的確保した領域を初期化
        node = (Node) {
                .action=action,
                .is_checkmate=is_checkmate,
                .visit_count=0,
                .win_count=0,
                .children=NULL,
                .child_len=0
        };
    }

    return node;
}


void destruct_node(Node *node) {
    // nodeのみを開放するデストラクタ

    free(node->children);
}


void destruct_all_node(Node *node) {
    // node以下のノードを全て解放するデストラクタ

    for (int i = 0; i < node->child_len; ++i)
        destruct_all_node(&node->children[i]);
    free(node->children);
}


Node *select_node(Node *root) {
    // モンテカルロ法におけるselectionを行う
    // rootの子ノードの中からQ値+コストが最大になるものを選び、そのアドレスを返す (UCTの場合)
    // もし子ノードがなければNULLを返す
    // もしQ値+コストが無限大となるものがあれば、それを返す

    if (root->children == NULL)
        return NULL;

    double priority[LEN_ACTIONS] = {};
    double max_priority = 0;
    int max_index = 0;
    for (int i = 0; i < root->child_len; ++i) {
        Node *child = &root->children[i];

        if (child->visit_count == 0)
            return child;

        priority[i] = (double) child->win_count / child->visit_count
                      + sqrt(2 * log(root->visit_count) / child->visit_count);

        if (priority[i] > max_priority) {
            max_index = i;
            max_priority = priority[i];
        }
    }

    return &root->children[max_index];
}


void expand(Node *root, const Game *game) {
    // rootに新たな子ノードを1つ加える (本当に1つ？文献によって違う)
    // 1つの場合の疑問点: 選び方はランダム？ それ以上新たに加えられる指手がない場合はスルー？
    // 1つではなく全ての場合はrootは葉ノードである必要がある
    // 全ての場合の疑問点: expansion後、各ノードのカウントはリセット？それともそのまま？
    // とりあえず、expansionは考えられる全ての場合を展開することにし、カウンタはリセットしないとする
    // なお、加えるNode構造体はcalloc関数により動的に確保する
    // チェックメイトの盤面が渡された場合はエラー

    assert(root->children == NULL);

    // rootから取り得る行動を全列挙
    Action all_actions[LEN_ACTIONS];
    int len_actions = get_all_actions_with_tfr(game, all_actions);
    assert(len_actions != 0);

    // children配列の動的確保
    root->children = (Node *) malloc(len_actions * sizeof(Node));
    root->child_len = len_actions;
    assert(root->children != NULL);

    // children配列を埋める
    for (int i = 0; i < len_actions; ++i)
        root->children[i] = create_node(all_actions[i], game);
}


int evaluate(const Game *game) {
    // 盤面gameからランダムにプレイアウトを行い勝敗を判定する
    // 自分が勝った場合1を、相手が勝った場合0を返す
    // 千日手どうしよう

    // 盤面のセーブ
    int saved_id = save(game);

    AI random_ai_1 = create_random_move_ai();  // 今から数えて先手
    AI random_ai_2 = create_random_move_ai();  // 今から数えて後手

    // ランダムプレイアウト (random_ai_1が自分の代わり)
    int winner = play(
            (Game *) game,
            (PlayerInterface *) &random_ai_1,
            (PlayerInterface *) &random_ai_2,
            false
    );

    // 盤面の復帰
    load((Game *) game, saved_id);

    return (winner == 1) ? 1 : 0;  // random_ai_1が勝てば自分の勝ち
}


int determine(Node *root) {
    // rootの子ノードの中から次に打つべき手を選んで、そのアドレスを返す

    assert(root->children != NULL);

    double max_priority = 0;
    int max_index = 0;
    for (int i = 0; i < root->child_len; ++i) {
        double priority = (double) root->children[i].win_count / root->children->visit_count;
        if (max_priority < priority) {
            max_index = i;
            max_priority = priority;
        }
    }

    return max_index;
}


int monte_carlo_tree_search(Node *root, const Game *game) {
    // モンテカルロ木探索を1回行い、勝った場合1を、負けた場合0を返す

    ++root->visit_count;
    int i_win;

    Node *child = select_node(root);
    if (child == NULL) {  // rootが葉ノードであった場合
        if (root->is_checkmate) {
            i_win = 0;
        } else if (root->visit_count >= EXPANSION_THRESHOLD) {
            expand(root, game);
            --root->visit_count;
            return monte_carlo_tree_search(root, game);
        } else {
            i_win = evaluate(game);
        }
    } else {  // rootが内部ノードであった場合
        do_action((Game *) game, child->action);
        int opponent_win = monte_carlo_tree_search(child, game);
        i_win = (opponent_win) ? 0 : 1;
        undo_action((Game *) game);
    }

    root->win_count += i_win;
    return i_win;
}


typedef struct tagMonteCarloAI {
    Action (*get_action)(struct tagMonteCarloAI *self, const Game *game);

    Node tree;
} MonteCarloAI;


Action get_action_from_mct(MonteCarloAI *self, const Game *game) {
    debug_print("player%d is thinking...", 2 - game->turn % 2);
    Action previous_action = get_previous_action(game);

    Node next_root;
    int next_root_index = -1;
    for (int i = 0; i < self->tree.child_len; ++i) {
        next_root = self->tree.children[i];
        if (action_equal(&previous_action, &next_root.action)) {
            next_root_index = i;
            break;
        }
    }

    if (next_root_index == -1) {
        destruct_all_node(&self->tree);
        self->tree = create_node((Action) {}, NULL);
    } else {
        for (int i = 0; i < self->tree.child_len; ++i) {
            if (i == next_root_index)
                continue;
            destruct_all_node(&self->tree.children[i]);
        }
        destruct_node(&self->tree);
        self->tree = next_root;
    }

    Game game_copy = clone(game, MAX_SIMULATION_TURN);
    for (int i = 0; i < 1000; ++i) {
        monte_carlo_tree_search(&self->tree, &game_copy);
    }

    next_root_index = determine(&self->tree);
    next_root = self->tree.children[next_root_index];
    for (int i = 0; i < self->tree.child_len; ++i) {
        if (i == next_root_index)
            continue;
        destruct_all_node(&self->tree.children[i]);
    }
    destruct_node(&self->tree);

    self->tree = next_root;
    return next_root.action;
}


MonteCarloAI create_monte_carlo_ai() {
    Node root = create_node((Action) {}, NULL);
    return (MonteCarloAI) {
            .get_action=get_action_from_mct,
            .tree=root
    };
}


// 以下デバッグ用のmain関数
int main() {
    MonteCarloAI ai1 = create_monte_carlo_ai();
    MonteCarloAI ai2 = create_monte_carlo_ai();

    Game game = create_game(MAX_TURN);
    int winner = play(&game, (PlayerInterface *) &ai1, (PlayerInterface *) &ai2, true);
    if (winner == 1) {
        puts("first player won");
    } else if (winner == -1) {
        puts("second player won");
    } else {
        puts("draw");
    }

    return 0;
}


#endif  /* MONTECARLO */