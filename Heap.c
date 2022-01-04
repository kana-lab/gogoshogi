#include "Heap.h"


/**
 * index1 及び index2 の位置にあるheapの要素を入れ替える
 */
static void heap_swap(Heap *heap, unsigned int index1, unsigned int index2) {
    PElement memo = heap->q[index1];
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
static void bubble_up(Heap *heap, unsigned int bubble_index) {
    while (bubble_index > 0 && heap->q[bubble_index]->value < heap->q[(bubble_index-1)/2]->value) {
        heap_swap(heap, bubble_index, (bubble_index-1)/2);
        bubble_index = (bubble_index-1) / 2;
    }
}


/**
 * bubble_downでは泡が子よりも大きい間ずっと沈ませ続ける
 * bubble_indexは、heap中の泡の位置を表す
 */
static void bubble_down(Heap *heap, unsigned int bubble_index) {
    while (2*bubble_index + 2 < heap->current_size) {
        unsigned int min_index;
        if(heap->q[bubble_index*2+1]->value <= heap->q[bubble_index*2+2]->value) {
            min_index = bubble_index * 2 + 1;
        } else {
            min_index = bubble_index * 2 + 2;
        }
        if(heap->q[bubble_index]->value > heap->q[min_index]->value) {
            heap_swap(heap, bubble_index, min_index);
            bubble_index = min_index;
        } else break;
    }
}
