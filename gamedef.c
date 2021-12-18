#include <stdio.h>
#include <stdarg.h>
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