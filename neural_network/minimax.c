#include "nn_shogi.c"


/*
ゲーム木を探索するAIを定義する.
*/


typedef struct __gtnode {
    // ゲーム木のノードを表す.
    Board b;
    bool is_first;
    double evaluation;
    struct __gtnode *parent;
    int len_children;
    struct __gtnode **children;
    Action action;
} GameTreeNode;


typedef struct {
    // 幅優先探索で用いるキュー
    GameTreeNode **data;
    int start;
    int end;
    int size;
} Queue;


void queue_init(Queue *self) {
    // キューを初期化する.
    self->size = 2;
    self->data = malloc(self->size * sizeof(GameTreeNode*));
    self->start = 0;
    self->end = 0;
}


void queue_resize(Queue *self) {
    // キューの長さを2倍にする.
    assert(self->start == self->end);
    GameTreeNode **old_data = self->data;
    self->data = malloc(2*self->size * sizeof(GameTreeNode*));
    for (int i = 0; i < self->size; i++) {
        self->data[i] = old_data[(self->start+i)%self->size];
    }
    free(old_data);
    self->start = 0;
    self->end = self->size;
    self->size *= 2;
}


void queue_push(Queue *self, GameTreeNode *gt) {
    // キューの最後尾に要素をpushする.
    self->data[self->end++] = gt;
    self->end %= self->size;
    if (self->start == self->end)
        // キューが埋まったときにresizeする.
        queue_resize(self);
}


int queue_is_empty(Queue *self) {
    return self->start == self->end;
}


void queue_free(Queue *self) {
    free(self->data);
}


GameTreeNode *queue_pop(Queue *self) {
    // キューの先頭から要素をpopする.
    if (queue_is_empty(self))
        return NULL;
    GameTreeNode *res = self->data[self->start];
    self->start++;
    self->start %= self->size;
    return res;
}


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

    // 時間計測の準備をする.
    struct timespec start_time, tmp_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    double max_time = MAX_TIME * 0.9;

    // パラメータを定義する.
    int max_children = 4; // 分岐数の最大値. これ以上の分岐は評価関数によってすぐに枝刈りを行う.
    
    // 変数を初期化する.
    Queue que;
    queue_init(&que);
    int len_array = 0;

    // 根を設定する.
    GameTreeNode *root = malloc(sizeof(GameTreeNode));
    Action action;
    gtnode_init(root, &game->current, game->turn%2, NULL, action, &self->nn, max_children);
    queue_push(&que, root);

    // BFSを行う.
    while (true) {

        clock_gettime(CLOCK_REALTIME, &tmp_time);
        if (max_time < stop_watch(start_time, tmp_time))
            // max_time以上の時間が経過しているとき
            break;
        
        if (queue_is_empty(&que))
            // キューが空のとき
            break;
        
        GameTreeNode *tmp = queue_pop(&que);
        gtnode_expand(tmp, &self->nn, max_children);
        for (int i = 0; i < tmp->len_children; i++)
            queue_push(&que, tmp->children[i]);
    }
    
    // 評価値を更新する.
    gtnode_update(root);

    // 最善手を選択する.
    int idx = gtnode_argmin(root->children, root->len_children);
    Action res = root->children[idx]->action;
    
    // 各指手の評価値を出力する.
    for (int i = 0; i < root->len_children; i++){
        Action action = root->children[i]->action;
        if (game->turn % 2 == 0)
            reverse_action(&action);
        char buffer[32];
        action_to_string(action, buffer);
        debug_print("%s %lf", buffer, 1.0-root->children[i]->evaluation);
    }
    
    // メモリを解放する.
    queue_free(&que);
    gtnode_free(root);

    // 思考時間を出力する.
    clock_gettime(CLOCK_REALTIME, &tmp_time);
    debug_print("thinking time: %lf s", stop_watch(start_time, tmp_time));

    return res;
}


NNAI create_minimax_ai(char load_file_name[]) {
    NNAI ai;
    ai.get_action = game_tree_search;
    nn_load_model(&ai.nn, load_file_name);
    return ai;
}
