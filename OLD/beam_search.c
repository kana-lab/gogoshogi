#include "nn_shogi.c"


/*
ビームサーチを行うAIを定義する.
多分どこかにバグがあって弱い.
*/


typedef struct __bsnode{
    // Beam Search で使用する構造体
    Board b;
    bool is_first;
    double evaluation;
    struct __bsnode *parent;
    int len_children;
    struct __bsnode **children;
} BeamNode;


void BeamNode_init(BeamNode *self, const Board *b, bool is_first, BeamNode *parent, NeuralNetwork *nn) {
    // BeamNode を初期化する.
    // 子ノードの探索は行わない.
    self->b = *b;
    self->is_first = is_first;
    self->evaluation = nn_evaluate(nn, is_first, b);
    self->parent = parent;
    self->len_children = -1; // 探索前は-1に設定する.
    self->children = malloc(LEN_ACTIONS * sizeof(BeamNode*));
}


void BeamNode_free(BeamNode *self) {
    // BeamNode のメモリを解放する.
    // 子ノードのメモリ解放も同時に行う.
    for (int i = 0; i < self->len_children; i++) {
        BeamNode_free(self->children[i]);
    }
    free(self->children);
    free(self);
}


int BeamNode_comparison(const void *bn1, const void *bn2) {
    // 評価値についての比較を行う.
    // bn1 <= bn2 のときに -1, そうでないときに 1 を返す.
    if ((*(BeamNode**)bn1)->evaluation < (*(BeamNode**)bn2)->evaluation)
        return -1;
    else
        return 1;
}


void BeamNode_expand(BeamNode *self, NeuralNetwork *nn, int max_children) {
    // BeamNode の子を探索する.
    Action all_actions[LEN_ACTIONS];
    self->len_children = get_useful_actions(&self->b, all_actions);
    for (int i = 0; i < self->len_children; i++) {
        Board b = self->b;
        update_board(&b, all_actions[i]);
        reverse_board(&b);
        BeamNode *child = malloc(sizeof(BeamNode));
        BeamNode_init(child, &b, 1-self->is_first, self, nn);
        self->children[i] = child;
    }
    // 子ノードを評価値順に並べ替える.
    qsort(self->children, self->len_children, sizeof(BeamNode*), BeamNode_comparison);
    // 子ノードの一部を削る.
    if (self->len_children > max_children)
        for (int i = max_children; i < self->len_children; i++)
            BeamNode_free(self->children[i]);
        self->len_children = max_children;
    // 親ノードの評価値を更新する.
    if (self->len_children == 0)
        self->evaluation = -1.0;
    else
        self->evaluation = 1.0 - self->children[0]->evaluation;
}


int BeamNode_argmax(BeamNode **array, int len_array) {
    // arrayの中で最も評価値が高いもののindexを返す.
    int best = 0;
    for (int i = 0; i < len_array; i++) {
        if (array[best]->evaluation < array[i]->evaluation)
            best = i;
    }
    return best;
}


int BeamNode_argmin(BeamNode **array, int len_array) {
    // arrayの中で最も評価値が低いもののindexを返す.
    int best = 0;
    for (int i = 0; i < len_array; i++) {
        if (array[i]->evaluation < array[best]->evaluation)
            best = i;
    }
    return best;
}


int beam_search_1step(BeamNode **array, int len_array, int width, NeuralNetwork *nn, int depth, int max_children) {
    // ビームサーチの1ステップを行う.
    // 次の探索ノードをarrayに代入する.
    // 返り値は次の探索ノードの数である.
    assert(len_array);

    // 子ノードを探索する.
    for (int i = 0; i < len_array; i++)
        BeamNode_expand(array[i], nn, max_children);
    
    // 次の探索ノードを列挙する.
    BeamNode **new_array = malloc(width * sizeof(BeamNode*));
    int len_new_array = 0;
    int *indices = calloc(len_array, sizeof(int));
    for (int i = 0; i < width; i++) {
        int best;
        if (depth % 2)
            best = BeamNode_argmin(array, len_array);
        else
            best = BeamNode_argmax(array, len_array);
        if (array[best]->evaluation == -1.0)
            break;
        new_array[len_new_array++] = array[best]->children[indices[best]++];
        if (array[best]->len_children == indices[best])
            array[best]->evaluation = -1.0;
        else
            array[best]->evaluation = 1.0 - array[best]->children[indices[best]]->evaluation;
    }

    // 枝刈りしたノードの評価値を2.0にする.
    for (int i = 0; i < len_array; i++) {
        for (int j = indices[i]; j < array[i]->len_children; j++)
            array[i]->children[j]->evaluation = 2.0;
    }

    // arrayを更新する.
    for (int i = 0; i < len_new_array; i++) {
        array[i] = new_array[i];
    }

    // メモリを解放する.
    free(new_array);
    free(indices);

    return len_new_array;
}


void BeamNode_update(BeamNode *self) {
    // 評価値の更新を子ノードも含めて行う.

    if (self->evaluation == 2.0) {
        // selfが枝刈りされたノードのとき
        assert(self->len_children == -1);
        return;
    } else if (self->len_children == 0) {
        // selfが詰みのとき
        self->evaluation = 0.0;
        return;
    } else if (self->len_children == -1)
        // selfが葉のとき
        return;
    
    // 子ノードの評価値を更新する.
    for (int i = 0; i < self->len_children; i++)
        BeamNode_update(self->children[i]);
    // 最善手を取得する.
    int idx = BeamNode_argmin(self->children, self->len_children);
    if (self->children[idx]->evaluation == 2.0)
        // 子ノードが全て枝刈りされているとき
        self->evaluation = 2.0;
    else
        self->evaluation = 1.0 - self->children[idx]->evaluation;
}


Action beam_search(NNAI *self, const Game *game) {
    // ビームサーチによって最善手を取得する.

    // パラメータを定義する.
    int width = 1000;
    int max_depth = 10;
    int max_children = 10;

    // 変数を初期化する.
    BeamNode **array = malloc(width * sizeof(BeamNode*));
    int len_array = 0;
    BeamNode *root = malloc(sizeof(BeamNode));
    BeamNode_init(root, &game->current, game->turn%2, NULL, &self->nn);
    array[len_array++] = root;

    // ビームサーチを行う.
    for (int depth = 0; depth < max_depth; depth++) {
        len_array = beam_search_1step(array, len_array, width, &self->nn, depth, max_children);
        if (len_array == 0)
            break;
    }
    
    // 評価値を更新する.
    BeamNode_update(root);

    // 最善手を選択する.
    int idx = BeamNode_argmin(root->children, root->len_children);
    Hash next_hash = encode(&root->children[idx]->b);

    Action all_actions[LEN_ACTIONS];
    int len_all_actions = get_useful_actions(&game->current, all_actions);
    int best_action = -1;
    for (int i = 0; i < len_all_actions; i++) {
        Board next_b = game->current;
        update_board(&next_b, all_actions[i]);
        reverse_board(&next_b);
        if (hash_equal(next_hash, encode(&next_b))) {
            best_action = i;
            break;
        }
    }
    if (best_action == -1)
        debug_print("cannot find the best action");

    // メモリを解放する.
    free(array);
    BeamNode_free(root);

    return all_actions[best_action];
}


NNAI create_beam_search_ai(char load_file_name[]) {
    NNAI ai;
    ai.get_action = beam_search;
    nn_load_model(&ai.nn, load_file_name);
    return ai;
}
