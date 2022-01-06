#ifndef MULTITHREAD_H
#define MULTITHREAD_H


#include <pthread.h>
#include "Game.h"

#define NUMBER_OF_THREADS      8        // スレッド数
#define MAX_GARBAGE_QUEUE_SIZE 1000000  // ゴミ(解放待ちのポインタ)を格納するキューのサイズ
#define INF_DEPTH              100000   // ゲーム木の深さが無限であることを表す値
#define DEPTH_STRIDE           3        // 1つのスレッドが一度に探索するゲーム木の深さ


typedef struct tagNode Node, *PNode;


/**
 * PNode型の要素を格納するヒープキュークラス & そのメソッド
 * ヒープの最大サイズを超えるような使い方はしないものとする
 */
typedef struct {
    PNode *buf;           // ヒープのバッファ
    size_t max_size;      // ヒープの最大サイズ
    size_t current_size;  // ヒープの現在の要素数
} Heap;

Heap construct_heap(size_t max_size);

void destruct_heap(Heap *heap);

void heap_delete(Heap *heap, size_t delete_index);

void heap_replace(Heap *heap, size_t replace_index, int replace_value);

void heap_push(Heap *heap, PNode node);


/**
 * ゲーム木のノードを表すクラス & そのメソッド
 */
struct tagNode {
    /* public */
    bool is_leaf;                            // このノードが葉であるか否か
    const Action action;                     // playerが取った行動
    const int player;                        // 行動actionを取ったプレイヤー
    Heap children;                           // 子ノードを入れるヒープ
    int value_for_heap;                      // 親ノードのヒープ中でソートに用いられる値
    PNode parent;                            // 親ノードへのポインタ

    /* private */
    volatile size_t index_in_parents_heap_;  // 親ノードのヒープのバッファ中でのインデックス
};

PNode construct_node(bool is_leaf, Action action, int player, PNode parent, size_t index_in_parents_heap);

void destruct_node(PNode node);

void destruct_node_recursively(PNode root);

size_t get_index_in_parents_heap(PNode self);

bool being_edited(PNode node);


/**
 * PNode型のゴミ(解放待ちのポインタ)、及びそれを格納するキューを表すクラス & それらのメソッド
 */
typedef struct {
    PNode root;               // ゴミである木の根
    size_t timing_of_delete;  // 削除のタイミング (-1なら即時、正なら適時、0は未定義)
} Garbage;

typedef struct {
    Garbage *buf;          // ゴミを入れるバッファ
    size_t max_size;       // キューの最大サイズ
    size_t start_index;    // リングバッファの開始インデックス
    size_t end_index;      // リングバッファの終端インデックス
    pthread_mutex_t lock;  // キューのロック
} GarbageQueue;

GarbageQueue construct_garbage_queue(size_t max_size);

void destruct_garbage_queue(GarbageQueue *queue);

/// キューからポップする、ポップする要素がない場合null_garbageを返す
Garbage garbage_queue_pop(GarbageQueue *queue);

/// キューにプッシュする、プッシュに成功したか否かを返す
bool garbage_queue_push(GarbageQueue *queue, Garbage garbage);

bool is_null_garbage(Garbage garbage);


/**
 * ゲーム木を探索する際に使用する共有リソースを表すクラス & そのメソッド
 */
typedef struct {
    /* public */
    const Game initial_game_state;          // Gameの最初の状態
    const Action action_history[MAX_TURN];  // 行動を全てメモしておくための配列
    GarbageQueue garbage_queue;             // ゴミを格納するキュー
    pthread_mutex_t game_tree_lock;         // ゲーム木の内部ノードの読み書きに関するロック

    /* private */
    volatile size_t action_index_;          // action_historyの要素の個数
    volatile PNode root_;                   // ゲーム木のルート
    volatile bool is_going_to_finish_;      // スレッドを止めるか否か
} SharedResources;

SharedResources *construct_shared_resources(const Game *initial_game_state, bool is_first_player);

void destruct_shared_resources(SharedResources *self);

size_t get_action_index(SharedResources *self);

PNode get_game_tree_root(SharedResources *self);

bool is_going_to_finish(SharedResources *self);


/**
 * スレッド1つ分を表すクラスExplorer / GarbageCollectorの宣言
 */
typedef struct {
    pthread_t thread_id;                // スレッドID
    SharedResources *shared_resources;  // 共有リソースへのポインタ
    size_t local_action_index;          // local_gameがどこまで進んでいるかを表すインデックス
    Game local_game;                    // ゲーム木の探索に用いるGameオブジェクト
} Explorer, *PExplorer;

typedef struct {
    pthread_t thread_id;                // スレッドID
    SharedResources *shared_resources;  // 共有リソースへのポインタ
    const PExplorer *p_explorers;       // ゲーム木の探索者を格納する配列
    const int number_of_explorers;      // ゲーム木の探索者の数
} GarbageCollector;

Explorer *construct_explorer(SharedResources *shared_resources);

GarbageCollector *construct_garbage_collector(
        SharedResources *shared_resources,
        const PExplorer *p_explorers,
        int number_of_explorers
);

void destruct_explorer(Explorer *self);

void destruct_garbage_collector(GarbageCollector *self);

/// スレッドに渡す関数であり、共有リソースにあるゲーム木を勝手に拡張する
void *explore(Explorer *self);

/// スレッドに渡す関数であり、共有リソースにあるGarbageQueue中のゴミを解放する
void *collect_garbage(GarbageCollector *self);


/**
 * マルチスレッドで必勝法を全探索するAIを表すクラス & そのメソッド
 */
typedef struct tagMultiExplorer {
    Action (*get_action)(struct tagMultiExplorer *self, const Game *game);

    SharedResources *shared_resources;
    Explorer *explorers[NUMBER_OF_THREADS];
    GarbageCollector *garbage_collector;

    // 暫定的な行動を優先度の高い順に格納する
    // 格納する際はロックを獲得すること
    // これ必要？他のAIとの統合のさせ方次第
    Action tmp_actions[LEN_ACTIONS];
    int tmp_actions_len;
    pthread_mutex_t tmp_actions_lock;
} MultiExplorer;

MultiExplorer create_multi_explorer(const Game *initial_game_state, bool is_first_player);

void destruct_multi_explorer(MultiExplorer *self);

/// メインスレッドで動作する関数であり、MultiExplorer.get_actionに代入される
Action determine_next_action(MultiExplorer *self, const Game *game);


#endif  /* MULTITHREAD_H */
