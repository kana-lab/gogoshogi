#include "nn_shogi.c"


/*
ゲーム木を探索するAIを定義する.
*/


typedef struct __gtnode{
    // ゲーム木のノードを表す.
    Board b;
    bool is_first;
    double evaluation;
    struct __gtnode *parent;
    int len_children;
    struct __gtnode **children;
    Action action;
} GameTreeNode;


void gtnode_init(GameTreeNode *self, const Board *b, bool is_first, GameTreeNode *parent, Action action, NeuralNetwork *nn, int max_children) {
    // GameTreeNodeを初期化する.
    // 子ノードの探索は行わない.
    self->b = *b;
    self->is_first = is_first;
    self->evaluation = nn_evaluate(nn, is_first, b);
    self->parent = parent;
    self->len_children = -1; // 探索前は-1に設定する.
    self->children = malloc(max_children * sizeof(GameTreeNode*));
    self->action = action;
}


void gtnode_free(GameTreeNode *self) {
    // GameTreeNodeのメモリを解放する.
    // 子ノードのメモリ解放も同時に行う.
    for (int i = 0; i < self->len_children; i++) {
        gtnode_free(self->children[i]);
    }
    free(self->children);
    free(self);
}


int gtnode_comparison(const void *gt1, const void *gt2) {
    // 評価値についての比較を行う.
    // gt1 <= gt2 のときに -1, そうでないときに 1 を返す.
    if ((*(GameTreeNode**)gt1)->evaluation < (*(GameTreeNode**)gt2)->evaluation)
        return -1;
    else
        return 1;
}


void gtnode_expand(GameTreeNode *self, NeuralNetwork *nn, int max_children) {
    // BeamNode の子を探索する.

    // 子ノードを取得する.
    Action all_actions[LEN_ACTIONS];
    int len_children = get_useful_actions(&self->b, all_actions);
    GameTreeNode **children = malloc(len_children * sizeof(GameTreeNode*));
    for (int i = 0; i < len_children; i++) {
        Board b = self->b;
        update_board(&b, all_actions[i]);
        reverse_board(&b);
        GameTreeNode *child = malloc(sizeof(GameTreeNode));
        gtnode_init(child, &b, 1-self->is_first, self, all_actions[i], nn, max_children);
        children[i] = child;
    }
    
    // 子ノードを評価値順に並べ替える.
    qsort(children, len_children, sizeof(GameTreeNode*), gtnode_comparison);

    // 評価値が低いものから順に, 最大max_children個をself->childrenに代入する.
    self->len_children = MIN(len_children, max_children);
    for (int i = 0; i < len_children; i++) {
        if (i < self->len_children)
            self->children[i] = children[i];
        else
            gtnode_free(children[i]);
    }
    free(children);

    // 親ノードの評価値を更新する.
    if (self->len_children == 0)
        self->evaluation = 0.0;
    else
        self->evaluation = 1.0 - self->children[0]->evaluation;
}


int gtnode_argmax(GameTreeNode **array, int len_array) {
    // arrayの中で最も評価値が高いもののindexを返す.
    int best = 0;
    for (int i = 0; i < len_array; i++) {
        if (array[best]->evaluation < array[i]->evaluation)
            best = i;
    }
    return best;
}


int gtnode_argmin(GameTreeNode **array, int len_array) {
    // arrayの中で最も評価値が低いもののindexを返す.
    int best = 0;
    for (int i = 0; i < len_array; i++) {
        if (array[i]->evaluation < array[best]->evaluation)
            best = i;
    }
    return best;
}


int search_1step(GameTreeNode **array, int len_array, NeuralNetwork *nn, int max_children) {
    // ゲーム木探索の1ステップを行う.
    // 次の探索ノードをarrayに代入する.
    // 返り値は次の探索ノードの数である.
    assert(len_array);

    // 子ノードを探索する.
    for (int i = 0; i < len_array; i++)
        gtnode_expand(array[i], nn, max_children);
    
    // 次の探索ノードを列挙する.
    GameTreeNode **new_array = malloc((len_array*max_children) * sizeof(GameTreeNode*));
    int len_new_array = 0;
    for (int i = 0; i < len_array; i++) {
        for (int j = 0; j < array[i]->len_children; j++)
            new_array[len_new_array++] = array[i]->children[j];
    }

    // arrayを更新する.
    for (int i = 0; i < len_new_array; i++) {
        array[i] = new_array[i];
    }

    // メモリを解放する.
    free(new_array);

    return len_new_array;
}


void gtnode_update(GameTreeNode *self) {
    // 評価値の更新を子ノードも含めて行う.
    
    if (self->len_children == -1)
        // selfが葉のとき
        return;
    else if (self->len_children == 0) {
        // selfが詰みの局面のとき
        assert(self->evaluation == 0.0);
        return;
    }
    
    // 子ノードの評価値を更新する.
    for (int i = 0; i < self->len_children; i++)
        gtnode_update(self->children[i]);
    
    // 最善手を取得する.
    int idx = gtnode_argmin(self->children, self->len_children);
    self->evaluation = 1.0 - self->children[idx]->evaluation;
}


Action game_tree_search(NNAI *self, const Game *game) {
    // Mini-Max法によって最善手を取得する.

    // パラメータを定義する.
    int max_depth = 5;
    int max_children = 4;
    
    // 変数を初期化する.
    GameTreeNode **array = malloc((int)pow(max_children,max_depth) * sizeof(GameTreeNode*));
    int len_array = 0;

    // 根を設定する.
    GameTreeNode *root = malloc(sizeof(GameTreeNode));
    Action action;
    gtnode_init(root, &game->current, game->turn%2, NULL, action, &self->nn, max_children);
    array[len_array++] = root;

    // 探索を行う.
    for (int depth = 0; depth < max_depth; depth++) {
        len_array = search_1step(array, len_array, &self->nn, max_children);
        if (len_array == 0)
            break;
    }
    
    // 評価値を更新する.
    gtnode_update(root);

    // 最善手を選択する.
    int idx = gtnode_argmin(root->children, root->len_children);
    Action res = root->children[idx]->action;

    for (int i = 0; i < root->len_children; i++){
        Action action = root->children[i]->action;
        reverse_action(&action);
        char buffer[32];
        action_to_string(action, buffer);
        printf("%s %lf, ", buffer, root->children[i]->evaluation);
    }

    // メモリを解放する.
    free(array);
    gtnode_free(root);

    return res;
}


NNAI create_minimax_ai(char load_file_name[]) {
    NNAI ai;
    ai.get_action = game_tree_search;
    nn_load_model(&ai.nn, load_file_name);
    return ai;
}
