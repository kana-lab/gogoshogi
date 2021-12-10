/*
main関数の一部を書きました。
色々と雑に書いています。

先手番と後手番で4つに場合分けをしているのはわざとです。
*/

#define MAX_TURN 150
#define FIRST 1
#define SECOND 0


Action get_user_action(int turn){
    //scanf
    //Action action = string_to_action
    if (turn % 2 == 0)
        // 後手番のとき
        reverse_action(action);
    return action;
}

void display_action(Action action, int turn){
    if (turn % 2 == 0)
        // 後手番のとき
        reverse_action(action);
    printf("%s", action_to_string(action));
}


int main(void){

    // 実行時引数の処理
    int first_is_user = 1;
    int second_is_user = 0;



    Board board = create_board();
    print_board_for_debug(&board);

    //history = ; 盤面の履歴
    int winner = -1;

    for (int turn = 1; turn <= MAX_TURN; turn++){
        Action action;

        if (turn % 2){
            // 先手番
            if (first_is_user)
                action = get_user_action(turn);
            else{
                action = get_ai_action(&board, history, turn);
                display_action(action);
            }
        }
        else{
            // 後手番
            if (second_is_user)
                action = get_user_action(turn);
            else{
                action = get_ai_action(&board, history, turn);
                display_action(action, turn);
            }
        }

        winner = move_piece(&board, action, history, turn);
        print_board_for_debug(&board);

        if (winner != -1)
            break;
        
        reverse_board(&board);
    }

    // 出力
}