#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "gamedef.h"


void debug_print(const char *msg, ...) {
    // デバッグプリント用の関数
    // ほぼprintf関数と変わらないが、末尾に改行が入る
    // #define DEBUG_MODE としてDEBUG_MODEを定義すればデバッグプリントが有効になる

#ifdef DEBUG_MODE
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    puts("");
    fflush(stdin);
    va_end(ap);
#endif  /* DEBUG_MODE */
}


double stop_watch(struct timespec start_time, struct timespec end_time) {
    // 経過時間を秒単位で返す.
    /* 使用例
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    // 2秒間処理を止める.
    sleep(2);
    clock_gettime(CLOCK_REALTIME, &end_time);
    printf("%lf\n", stop_watch(start_time, end_time)); // 2.0程度の値が出力される.
    */
    long long int sec = end_time.tv_sec - start_time.tv_sec;
    long long int nsec = end_time.tv_nsec - start_time.tv_nsec;
    return (double)sec + (double)nsec / (1000 * 1000 * 1000);
}
