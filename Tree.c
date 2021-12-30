#include "Tree.h"


/**
 * index1 及び index2 の位置にあるheapの要素を入れ替える
 */
static void heap_swap(Heap *heap, unsigned int index1, unsigned int index2) {
    Element *memo = heap->q[index1];
    heap->q[index1] = heap->q[index2];
    heap->q[index2] = memo;

    heap->q[index1]->index_in_heap = index1;
    heap->q[index2]->index_in_heap = index2;
}


/**
 * 既に述べた2つの参考文献にあるように、delete と replace の後の整合性の調整は
 * バブルアップ(swim)とバブルダウン(sink)によって行われる
 * bubble_upでは、泡が親よりも小さい間ずっと浮上させ続ける
 * bubble_indexは、heap中の泡の位置を表す
 */
static void bubble_up(Heap *heap, unsigned int bubble_index);


/**
 * bubble_downでは泡が子よりも大きい間ずっと沈ませ続ける
 * bubble_indexは、heap中の泡の位置を表す
 */
static void bubble_down(Heap *heap, unsigned int bubble_index);