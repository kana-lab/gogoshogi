#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "MultiThread.h"


Heap construct_heap(size_t max_size) {
    assert(max_size != 0);
    return (Heap) {
            .buf=(PNode *) malloc(max_size * sizeof(PNode)),
            .max_size=max_size,
            .current_size=0
    };
}


void destruct_heap(Heap *heap) {
    free(heap->buf);
    heap->buf = NULL;
}


/**
 * index1 及び index2 の位置にあるheapの要素を入れ替える
 */
static void heap_swap_(Heap *heap, size_t index1, size_t index2) {
    PNode memo = heap->buf[index1];
    heap->buf[index1] = heap->buf[index2];
    heap->buf[index2] = memo;

    heap->buf[index1]->index_in_parents_heap_ = index1;
    heap->buf[index2]->index_in_parents_heap_ = index2;
}


/**
 * 既に述べた2つの参考文献にあるように、delete と replace の後の整合性の調整は
 * バブルアップ(swim)とバブルダウン(sink)によって行われる
 * bubble_upでは、泡が親よりも小さい間ずっと浮上させ続ける
 * bubble_indexは、heap中の泡の位置を表す
 */
static void bubble_up_(Heap *heap, size_t bubble_index) {
    while (bubble_index > 0 &&
           heap->buf[bubble_index]->value_for_heap < heap->buf[(bubble_index - 1) / 2]->value_for_heap) {
        heap_swap_(heap, bubble_index, (bubble_index - 1) / 2);
        bubble_index = (bubble_index - 1) / 2;
    }
}


/**
 * bubble_downでは泡が子よりも大きい間ずっと沈ませ続ける
 * bubble_indexは、heap中の泡の位置を表す
 */
static void bubble_down_(Heap *heap, size_t bubble_index) {
    while (2 * bubble_index + 1 < heap->current_size) {
        size_t min_index;
        if (2 * bubble_index + 2 < heap->current_size) {
            if (heap->buf[bubble_index * 2 + 1]->value_for_heap <= heap->buf[bubble_index * 2 + 2]->value_for_heap) {
                min_index = bubble_index * 2 + 1;
            } else {
                min_index = bubble_index * 2 + 2;
            }
        } else {
            min_index = bubble_index * 2 + 1;
        }
        if (heap->buf[bubble_index]->value_for_heap >= heap->buf[min_index]->value_for_heap) {
            heap_swap_(heap, bubble_index, min_index);
            bubble_index = min_index;
        } else break;
    }
}


void heap_delete(Heap *heap, size_t delete_index) {
    if (delete_index == heap->current_size - 1) {
        heap->buf[heap->current_size - 1] = NULL;
        --heap->current_size;
    } else {
        heap->buf[delete_index] = heap->buf[heap->current_size - 1];
        heap->buf[delete_index]->index_in_parents_heap_ = delete_index;
        heap->buf[heap->current_size - 1] = NULL;
        heap->current_size--;
        bubble_up_(heap, delete_index);
        bubble_down_(heap, delete_index);
    }
}


void heap_replace(Heap *heap, size_t replace_index, int replace_value) {
    heap->buf[replace_index]->value_for_heap = replace_value;
    bubble_up_(heap, replace_index);
    bubble_down_(heap, replace_index);
}


void heap_push(Heap *heap, PNode node) {
    heap->buf[heap->current_size++] = node;
    bubble_up_(heap, heap->current_size - 1);
}


PNode construct_node(bool is_leaf, Action action, int player, PNode parent, size_t index_in_parents_heap) {
    PNode res = (PNode) malloc(sizeof(Node));

    Node node = (Node) {
            .parent=parent,
            .index_in_parents_heap_=index_in_parents_heap,
            .is_leaf=is_leaf,
            .action=action,
            .player=player,
            .children={},
            .value_for_heap=0,
    };

    memcpy(res, &node, sizeof(Node));

    return res;
}


void destruct_node(PNode node) {
    destruct_heap(&node->children);
    node->parent = NULL;
    free(node);
}


void destruct_node_recursively(PNode root) {
    if (root->is_leaf) {
        assert(root->value_for_heap == 0 || root->value_for_heap == INF_DEPTH);
        destruct_node(root);
    } else {
        for (size_t i = 0; i < root->children.current_size; ++i)
            destruct_node_recursively(root->children.buf[i]);
        destruct_node(root);
    }
}


size_t get_index_in_parents_heap(PNode self) {
    return self->index_in_parents_heap_;
}


bool being_edited(PNode node) {
    return (node->is_leaf) && (node->value_for_heap != 0 && node->value_for_heap != INF_DEPTH);
}


GarbageQueue construct_garbage_queue(size_t max_size) {
    ++max_size;

    GarbageQueue q = {
            .buf=(Garbage *) malloc(max_size * sizeof(Garbage)),
            .max_size=max_size,
            .start_index=0,
            .end_index=0,
            .lock=PTHREAD_MUTEX_INITIALIZER
    };

    assert(q.buf != NULL);

    return q;
}


void destruct_garbage_queue(GarbageQueue *queue) {
    assert(queue->start_index == queue->end_index);
    pthread_mutex_lock(&queue->lock);
    free(queue->buf);
    queue->buf = NULL;
    pthread_mutex_destroy(&queue->lock);
}


Garbage garbage_queue_pop(GarbageQueue *queue) {
    pthread_mutex_lock(&queue->lock);

    if (queue->start_index == queue->end_index) {
        pthread_mutex_unlock(&queue->lock);
        return (Garbage) {};
    }

    Garbage res = queue->buf[queue->start_index];
    queue->start_index = (queue->start_index + 1) % queue->max_size;

    pthread_mutex_unlock(&queue->lock);

    return res;
}


bool garbage_queue_push(GarbageQueue *queue, Garbage garbage) {
    pthread_mutex_lock(&queue->lock);

    size_t increased_end_index = (queue->end_index + 1) % queue->max_size;
    if (queue->start_index == increased_end_index) {
        pthread_mutex_unlock(&queue->lock);
        return false;
    }

    queue->buf[queue->end_index] = garbage;
    queue->end_index = increased_end_index;

    pthread_mutex_unlock(&queue->lock);

    return true;
}


bool is_null_garbage(Garbage garbage) {
    return (garbage.root == NULL) && (garbage.timing_of_delete == 0);
}


SharedResources *construct_shared_resources(const Game *initial_game_state, bool is_first_player) {
    SharedResources shared_resources = (SharedResources) {
            .initial_game_state=clone(initial_game_state, initial_game_state->max_turn),
            .action_history={},
            .garbage_queue=construct_garbage_queue(MAX_GARBAGE_QUEUE_SIZE),
            .game_tree_lock=PTHREAD_MUTEX_INITIALIZER,
            .action_index_=0,
            .root_=construct_node(true, (Action) {}, (is_first_player) ? -1 : 1, NULL, 0),
            .is_going_to_finish_=false
    };

    SharedResources *res = (SharedResources *) malloc(sizeof(SharedResources));
    memcpy(res, &shared_resources, sizeof(SharedResources));

    return res;
}


void destruct_shared_resources(SharedResources *self) {
//    assert(self->is_going_to_finish_);
    pthread_mutex_lock(&self->game_tree_lock);
    destruct_garbage_queue(&self->garbage_queue);
    destruct_game((Game *) &self->initial_game_state);
    destruct_node_recursively(self->root_);
    pthread_mutex_destroy(&self->game_tree_lock);
    free(self);
}


size_t get_action_index(SharedResources *self) {
    return self->action_index_;
}


PNode get_game_tree_root(SharedResources *self) {
    return self->root_;
}


bool is_going_to_finish(SharedResources *self) {
    return self->is_going_to_finish_;
}


Explorer *construct_explorer(SharedResources *shared_resources) {
    Explorer *self = (Explorer *) malloc(sizeof(Explorer));
    *self = (Explorer) {
            .shared_resources=shared_resources,
            .local_action_index=0,
            .local_game=clone(
                    &shared_resources->initial_game_state,
                    shared_resources->initial_game_state.max_turn
            ),
    };

    pthread_create(&self->thread_id, NULL, (void *) explore, self);

    return self;
}


GarbageCollector *construct_garbage_collector(
        SharedResources *shared_resources,
        const PExplorer p_explorers[NUMBER_OF_THREADS],
        int number_of_explorers
) {
    GarbageCollector *self = (GarbageCollector *) malloc(sizeof(GarbageCollector));
    GarbageCollector garbage_collector = (GarbageCollector) {
            .shared_resources=shared_resources,
            .number_of_explorers=number_of_explorers
    };

    memcpy(self, &garbage_collector, sizeof(GarbageCollector));

    for (size_t i = 0; i < NUMBER_OF_THREADS; ++i) {
        PExplorer *tmp = (PExplorer *) &self->p_explorers[i];
        *tmp = p_explorers[i];
    }

    pthread_create(&self->thread_id, NULL, (void *) collect_garbage, self);

    return self;
}


void destruct_explorer(Explorer *self) {
    // explorerをdestructする前にスレッドが終了する目処が立っている必要あり
    // すなわち、shared_resources.is_going_to_finish = true となっているべき
    // shared_resourcesは解放しない

    pthread_join(self->thread_id, NULL);
    destruct_game(&self->local_game);
}


void destruct_garbage_collector(GarbageCollector *self) {
    // garbage_collectorをdestructする前にスレッドが終了する目処が立っている必要あり
    // すなわち、shared_resources.is_going_to_finish = true となっているべき

    pthread_join(self->thread_id, NULL);
}


// thread-safe
static void free_nodes_from_leaves_(PNode root) {
    // root以下のゲーム木を帰りがけ順に解放

    root->parent = NULL;

    while (being_edited(root));

    if (root->is_leaf) {
        destruct_node(root);
    } else {
        for (size_t i = 0; i < root->children.current_size; ++i)
            free_nodes_from_leaves_(root->children.buf[i]);
        destruct_node(root);
    }
}


void *collect_garbage(GarbageCollector *self) {
    for (;;) {
        Garbage garbage = garbage_queue_pop(&self->shared_resources->garbage_queue);

        if (is_null_garbage(garbage)) {
            if (is_going_to_finish(self->shared_resources)) {
                pthread_exit(NULL);
            } else {
                usleep(50000);  // 50 ms
                continue;
            }
        }

        if (garbage.timing_of_delete != -1) {  // unsignedの値と-1の比較、あまり良くない
            size_t minimum_action_index = garbage.timing_of_delete;

            for (;;) {
                for (size_t i = 0; i < NUMBER_OF_THREADS; ++i) {
                    if (self->p_explorers[i]->local_action_index < minimum_action_index)
                        goto CONTINUE_ACTION_INDEX_CHECK;
                }

                /* else */
                break;

                CONTINUE_ACTION_INDEX_CHECK:;
            }

            destruct_node_recursively(garbage.root);
        } else {
            // 葉から解放することでバックプロパゲーションによる不具合を防ぐ
            // 編集中のノードはバックプロパゲーションが完了するまでbeing_edited == trueであり
            // free_node_from_leaves_ではその待ち合わせを行うため
            free_nodes_from_leaves_(garbage.root);
        }
    }
}


MultiExplorer create_multi_explorer(const Game *initial_game_state, bool is_first_player, char *nn_filename) {
    MultiExplorer multi_explorer = {
            .tmp_actions={},
            .tmp_actions_len=0,
            .neural_network=(NeuralNetwork *) malloc(sizeof(NeuralNetwork)),
            .first_call_flag_=is_first_player
    };
    multi_explorer.get_action = determine_next_action;
    multi_explorer.shared_resources = construct_shared_resources(initial_game_state, is_first_player);
    nn_load_model(multi_explorer.neural_network, nn_filename);

    for (size_t i = 0; i < NUMBER_OF_THREADS; ++i)
        multi_explorer.explorers[i] = construct_explorer(multi_explorer.shared_resources);

    multi_explorer.garbage_collector = construct_garbage_collector(
            multi_explorer.shared_resources,
            multi_explorer.explorers,
            NUMBER_OF_THREADS
    );

    return multi_explorer;
}


void destruct_multi_explorer(MultiExplorer *self) {
    self->shared_resources->is_going_to_finish_ = true;

    for (size_t i = 0; i < NUMBER_OF_THREADS; ++i)
        destruct_explorer(self->explorers[i]);
    destruct_garbage_collector(self->garbage_collector);

    assert(self->shared_resources->garbage_queue.start_index == self->shared_resources->garbage_queue.end_index);
    destruct_shared_resources(self->shared_resources);

    nn_free(self->neural_network);
    free(self->neural_network);
}


// thread-safe
static void change_root_(SharedResources *self, Action previous_action) {
    pthread_mutex_lock(&self->game_tree_lock);

    PNode current_root = self->root_;
    PNode next_root = NULL;
    for (size_t i = 0; i < current_root->children.current_size; ++i) {
        if (action_equal(&previous_action, &current_root->children.buf[i]->action)) {
            next_root = current_root->children.buf[i];
            heap_delete(&current_root->children, i);
            break;
        }
    }

    if (next_root == NULL) {
        next_root = construct_node(true, previous_action, current_root->player * (-1), NULL, 0);
    } else {
        next_root->parent = NULL;
    }

    bool success = garbage_queue_push(
            &self->garbage_queue,
            (Garbage) {.root=current_root, .timing_of_delete=self->action_index_ + 1}
    );
    assert(success);
    self->root_ = next_root;

    Action *const action_history = (Action *) self->action_history;
    action_history[self->action_index_] = previous_action;
    ++self->action_index_;

    pthread_mutex_unlock(&self->game_tree_lock);
}


size_t count_node_(PNode root) {
    if (root->is_leaf) {
        if (root->value_for_heap != 0 && root->value_for_heap != INF_DEPTH) {
            return root->value_for_heap;
        } else {
            return 1;
        }
    } else {
        size_t sum_ = 0;
        for (size_t i = 0; i < root->children.current_size; ++i)
            sum_ += count_node_(root->children.buf[i]);
        return sum_;
    }
}


Action determine_next_action(MultiExplorer *self, const Game *game) {
    SharedResources *const rsc = self->shared_resources;

    if (!self->first_call_flag_) {
        const Action previous_action = get_previous_action(game);
        change_root_(rsc, previous_action);
    } else {
        self->first_call_flag_ = false;
    }

//    {
//        self->tmp_actions_len = get_perfectly_useful_actions_with_tfr(game, self->tmp_actions);
//        assert(self->tmp_actions_len != 0);
//        sleep(9);
//    }

    // ここで9秒消費される
    self->tmp_actions_len = get_prioritized_actions(self->neural_network, game, self->tmp_actions);

    debug_print("garbage count: %ld, total released garbage: %ld",
                rsc->garbage_queue.end_index - rsc->garbage_queue.start_index,
                rsc->garbage_queue.start_index);

    pthread_mutex_lock(&rsc->game_tree_lock);
    debug_print("total number of searched nodes: %ld", count_node_(rsc->root_));

    Action next_action;
    for (size_t i = 0; i < rsc->root_->children.current_size; ++i) {
        if (rsc->root_->children.buf[i]->value_for_heap == INF_DEPTH) {
            debug_print("CONGRATULATION! MultiExplorer will win!");
            next_action = rsc->root_->children.buf[i]->action;
            goto NEXT_ACTION_FOUND;
        }
    }

    /* else */
    for (size_t i = 0; i < self->tmp_actions_len; ++i) {
        next_action = self->tmp_actions[i];
        for (size_t j = 0; j < rsc->root_->children.current_size; ++j)
            if (action_equal(&next_action, &rsc->root_->children.buf[j]->action))
                goto NEXT_ACTION_FOUND;
    }

    /* else (corner case) */
    next_action = self->tmp_actions[0];

    NEXT_ACTION_FOUND:
    pthread_mutex_unlock(&rsc->game_tree_lock);

    change_root_(rsc, next_action);
    return next_action;
}

/* version 1 */

//int calc_value_for_heap_(PNode node) {
//    if (node->is_leaf)
//        return node->value_for_heap;
//
//    int ret_val = node->children.buf[0]->value_for_heap + 1;
//    if (ret_val > INF_DEPTH)
//        ret_val = INF_DEPTH;
//
//    return ret_val;
//}

/* version 2 */

int calc_value_for_heap_(PNode node) {
    if (node->is_leaf)
        return node->value_for_heap;

    assert(node->children.current_size != 0);
    int ret_value = 0;
    for (size_t i = 0; i < node->children.current_size; ++i)
        if (node->children.buf[i]->value_for_heap != INF_DEPTH)
            ret_value += node->children.buf[i]->value_for_heap + 1;

    if (ret_value == 0)
        ret_value = INF_DEPTH;
    return ret_value;
}


PNode get_next_node_unsafe_(Explorer *self, PNode current_node) {
    PNode ret;

    if (current_node->is_leaf) {
        if (!being_edited(current_node) && current_node->value_for_heap != INF_DEPTH) {
            // at this point, being_edited(current_node) becomes true.
            current_node->value_for_heap += DEPTH_STRIDE;
            ret = current_node;
        } else {  // current_node is now being edited.
            ret = NULL;
        }
    } else {
        assert(current_node->children.current_size != 0);
        PNode child = current_node->children.buf[0];
        int old_value_for_heap = child->value_for_heap;

        do_action(&self->local_game, child->action);
        ret = get_next_node_unsafe_(self, child);

        if (ret == NULL) {
            undo_action(&self->local_game);
        } else {
            assert(get_index_in_parents_heap(child) == 0);
            int new_value_for_heap = child->value_for_heap;
            if (old_value_for_heap != new_value_for_heap) {
                heap_replace(&current_node->children, 0, child->value_for_heap);
                current_node->value_for_heap = calc_value_for_heap_(current_node);
            }
        }
    }

    return ret;
}


PNode get_next_node_(Explorer *self) {
    pthread_mutex_lock(&self->shared_resources->game_tree_lock);

    // shared_resources.root_の不整合を防ぐ
    if (self->local_action_index != get_action_index(self->shared_resources)) {
        pthread_mutex_unlock(&self->shared_resources->game_tree_lock);
        return NULL;
    }

    PNode ret = get_next_node_unsafe_(self, get_game_tree_root(self->shared_resources));

    pthread_mutex_unlock(&self->shared_resources->game_tree_lock);
    return ret;
}


bool update_action_index_(Explorer *self) {
    size_t new_action_index = get_action_index(self->shared_resources);
    if (self->local_action_index == new_action_index)
        return false;

//    debug_print("in thread %ld: updated action_index from %ld to %ld",
//                self->thread_id,
//                self->local_action_index,
//                new_action_index);

    for (size_t i = self->local_action_index; i < new_action_index; ++i)
        do_action(&self->local_game, self->shared_resources->action_history[i]);

    self->local_action_index = new_action_index;
    return true;
}


int expand_(PNode leaf, const Game *game, GarbageQueue *garbage_queue, int depth) {
    // leaf.is_leafは変化させないことに注意
    // leaf.value_for_heapは変化させないことに注意 (後で調整の必要あり)

    assert(depth > 0);
    const int current_player = leaf->player * (-1);

    Action all_actions[LEN_ACTIONS];
    const int action_len = get_perfectly_useful_actions_with_tfr(game, all_actions);
    assert(action_len != 0);

    if (action_len == -1) {  // 詰みの一手の場合
        PNode child = construct_node(true, all_actions[0], current_player, leaf, 0);
        leaf->children = construct_heap(1);
        heap_push(&leaf->children, child);

        if (current_player == 1) {  // 自分の勝ち
            child->value_for_heap = INF_DEPTH;
            return 0;  // normal state
        } else {  // 相手の勝ち
            return 1;  // delete state
        }
    } else if (depth != 1) {
        assert(action_len > 0);

        PNode children[LEN_ACTIONS];
        int child_len = 0;
        int ret_code = 0;

        for (int i = 0; i < action_len; ++i) {
            PNode child = construct_node(true, all_actions[i], current_player, leaf, 0);

            do_action((Game *) game, all_actions[i]);
            int status = expand_(child, game, garbage_queue, depth - 1);
            undo_action((Game *) game);

            child->is_leaf = false;
            child->value_for_heap = calc_value_for_heap_(child);
            children[child_len++] = child;

            if (status == 0) {  // normal state
                continue;
            } else {  // delete state
                assert(status == 1);

                if (current_player == -1) {  // 相手の手番なら全削除
                    ret_code = 1;
                    break;
                } else {  // 自分の手番なら必要最小限の削除
                    assert(current_player == 1);
                    if (action_len == 1) {
                        ret_code = 1;
                        break;
                    } else {
                        --child_len;
                        garbage_queue_push(
                                garbage_queue,
                                (Garbage) {.timing_of_delete=-1, .root=child}
                        );
                    }
                }
            }
        }

        if (child_len == 0) {
            children[child_len++] = construct_node(true, (Action) {}, current_player, leaf, 0);
            ret_code = 1;
        }

        leaf->children = construct_heap(child_len);
        for (int i = 0; i < child_len; ++i)
            heap_push(&leaf->children, children[i]);

        return ret_code;
    } else {  // depth == 1
        leaf->children = construct_heap(action_len);
        for (int i = 0; i < action_len; ++i)
            leaf->children.buf[i] = construct_node(true, all_actions[i], current_player, leaf, i);
        leaf->children.current_size = action_len;
        return 0;  // normal state
    }
}


// thread-unsafe
void value_for_heap_propagation_(PNode node) {
    PNode parent = node->parent;
    if (parent == NULL)
        return;

    heap_replace(&parent->children, get_index_in_parents_heap(node), calc_value_for_heap_(node));

    int new_value_for_heap = calc_value_for_heap_(parent);
    if (parent->value_for_heap != new_value_for_heap)
        value_for_heap_propagation_(parent);
}


// thread-unsafe
int delete_propagation_(SharedResources *rsc, PNode node) {
    // 戻り値は、既に削除されているノードを削除しようとした場合1、
    // 削除に成功した場合0、現在のルートノードを削除しようとした場合-1である
    // -1が返った場合必敗状態にある

    if (node->parent == NULL) {
        if (node == rsc->root_) {
            return -1;  // ルートノードを削除しようとした
        } else {
            return 1;  // 既に削除されているノードを削除しようとした
        }
    }

    if (node->player == 1) {
        if (node->parent->children.current_size == 1) {
            return delete_propagation_(rsc, node->parent);
        } else {
            heap_delete(&node->parent->children, node->index_in_parents_heap_);
            node->parent = NULL;
            garbage_queue_push(&rsc->garbage_queue, (Garbage) {node, -1});
            return 0;
        }
    } else {
        return delete_propagation_(rsc, node->parent);
    }
}


void stun_and_wait_opponents_mistake_(Explorer *self, PNode problematic_leaf) {
    size_t current_action_index = self->local_action_index;

    while (!is_going_to_finish(self->shared_resources)) {
        while (current_action_index == get_action_index(self->shared_resources))
            usleep(50000);  // 50 ms

        PNode new_root = get_game_tree_root(self->shared_resources);
        for (PNode node = problematic_leaf; node; node = node->parent) {
            if (node == new_root)
                goto LOOP_CONTINUE;
        }

        /* else */
        break;

        LOOP_CONTINUE:
        current_action_index = get_action_index(self->shared_resources);
    }
}


void *explore(Explorer *self) {
    while (!is_going_to_finish(self->shared_resources)) {
        update_action_index_(self);

        int saved_id = save(&self->local_game);

        PNode leaf = get_next_node_(self);
        if (leaf == NULL)
            continue;

        int ret_code = expand_(
                leaf, &self->local_game, &self->shared_resources->garbage_queue, DEPTH_STRIDE
        );

        load(&self->local_game, saved_id);

        // leaf.is_leaf must be true
        // leaf.value_for_heap must NOT be 0
        // these mean being_edited(leaf) == true

        if (ret_code == 1) {
            // 削除のバックプロパゲーションが必要
            pthread_mutex_lock(&self->shared_resources->game_tree_lock);
            int status = delete_propagation_(self->shared_resources, leaf);
            pthread_mutex_unlock(&self->shared_resources->game_tree_lock);

            // 必敗状態であり、これ以上の探索は無駄である
            // よって、スタンしつつ相手がミスするのを待つ
            if (status == -1) {
                debug_print("THAT'S A PITY. MultiExplorer will lose.");
                debug_print("thread %ld began to stun...", self->thread_id);
                stun_and_wait_opponents_mistake_(self, leaf);
                debug_print("thread %ld returned from stun.", self->thread_id);
            }
        } else if (leaf->value_for_heap != calc_value_for_heap_(leaf)) {
            pthread_mutex_lock(&self->shared_resources->game_tree_lock);
            value_for_heap_propagation_(leaf);
            pthread_mutex_unlock(&self->shared_resources->game_tree_lock);
        }

        // at this point, being_edited(leaf) becomes false
        leaf->is_leaf = false;
    }

    pthread_exit(NULL);
}