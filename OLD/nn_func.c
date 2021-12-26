void board_to_vector585(const Board *b, double vec[INPUT_SIZE]) {
    // 盤面を585次元のベクトルに変換する.
    assert(INPUT_SIZE == 585);

    // vecを初期化する.
    for (int i = 0; i < INPUT_SIZE; i++)
        vec[i] = 0.0;

    // 盤上の情報を入力する.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            vec[21 * (5 * i + j) + b->board[i][j] + MAX_PIECE_NUMBER] = 1.0;
    }

    // 持ち駒の情報を入力する.
    for (int i = 0; i < 5; i++) {
        vec[21 * 25 + i] = (double) b->next_stock[i + 1];
        vec[21 * 25 + 5 + i] = (double) b->previous_stock[i + 1];
    }

    // 自分の駒のききの数を入力する.
    double counts[5][5];
    count_connections(b, counts);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            vec[21 * 25 + 10 + 5 * i + j] = counts[i][j];
    }

    // 相手の駒のききの数を入力する.
    // 座標が180度ずれているが, 気にしないことにする.
    Board b_copy = *b;
    reverse_board(&b_copy);
    count_connections(&b_copy, counts);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            vec[21 * 25 + 10 + 25 + 5 * i + j] = counts[i][j];
    }
}


void count_connections(const Board *b, double counts[5][5]) {
    /*
    各マスについて, ききのある駒の数を数える.
    評価関数の入力に用いる.
    */

    // 初期化する.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++)
            counts[i][j] = 0.0;
    }

    // 数える.
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int piece = b->board[i][j];
            if (piece <= 0)
                // 自分の駒がないとき
                continue;
            if (piece != KAKU && piece != HISHA) {
                // 駒が角や飛ではないときに, 周囲に動く手を列挙する.
                for (int k = 0; k < move_length[piece]; k++) {
                    int x = i + move_matrix_x[piece][k];
                    int y = j + move_matrix_y[piece][k];
                    if (0 <= x && x < 5 && 0 <= y && y < 5) {// && b->board[x][y] <= EMPTY) {
                        counts[x][y] += 1.0;
                    }
                }
            }
            if (piece % NARI == KAKU || piece % NARI == HISHA) {
                // 角, 馬, 飛, 龍が直線的に動く手を列挙する.
                for (int k = 0; k < move_length[piece % NARI]; k++) {
                    int x = i + move_matrix_x[piece % NARI][k];
                    int y = j + move_matrix_y[piece % NARI][k];
                    while (0 <= x && x < 5 && 0 <= y && y < 5) {// && b->board[x][y] <= EMPTY) {
                        counts[x][y] += 1.0;
                        // 駒を飛び越えてききがあることにする.
                        /*
                        if (b->board[x][y] < EMPTY)
                            // 相手の駒を取ったとき
                            break;
                        */
                        x += move_matrix_x[piece % NARI][k];
                        y += move_matrix_y[piece % NARI][k];
                    }
                }
            }
        }
    }
}

