#ifndef HEAP_H
#define HEAP_H


typedef struct {                 // ヒープに入れる要素を表す構造体
    int value;                   // この値が小さい順になるようにヒープを作る事
    unsigned int index_in_heap;  // ヒープ中での自分のインデックスを保持しておく
} *PElement;


typedef struct {                // ヒープを表す構造体
    PElement *q;                // ヒープ本体、PElement型の要素の配列
    unsigned int max_size;      // ヒープの最大サイズ
    unsigned int current_size;  // 現在ヒープに入っている要素の個数
} Heap;


/**
 * delete_indexで表される場所にある要素 (すなわちheap->q[delete_index]) を削除する
 * 削除した後は次の参考文献:
 *     http://www.mathcs.emory.edu/~cheung/Courses/171/Syllabus/9-BinTree/heap-delete.html
 * のやり方に従ってヒープの整合性を取り戻す
 * ヒープは最小の要素がheap->q[0]となるようにする事 (参考文献と違うので注意)
 */
void heap_delete(Heap *heap, unsigned int delete_index) {
    heap->q[delete_index]->value = heap->q[current_size-1]->value;
    heap->q[delete_index]->index_in_heap = delete_index;
    heap->q[current_size-1] = NULL;
    current_size--;
    bubble_down(heap, delete_index);
}

/**
 * replace_indexで表される場所にある要素のvalue (すなわちheap->q[replace_index]->value) を
 * replace_valueで置き換え、次の参考文献:
 *     https://stackoverflow.com/questions/20397674/replacing-element-in-min-heap
 * の回答のうちの Bernhard Barker 氏のやり方に従ってヒープの整合性を取り戻す
 * ヒープは最小の要素がheap->q[0]となるようにする事 (参考文献と違うので注意)
 */
void heap_replace(Heap *heap, unsigned int replace_index, int replace_value){
    heap->q[replace_index]->value = replace_value;
    bubble_up(heap, replace_index);
}

#endif  /* HEAP_H */
