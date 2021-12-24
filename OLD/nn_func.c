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
