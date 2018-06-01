#include <stdio.h>
#include "game.h"


void game()
{
  char win = 0;
  char board[ROW][COL];
  InitBoard(board, ROW, COL);
  Printf_Board(board, ROW, COL);

  //游戏开始
  while (1) {
    PlayerMove(board, ROW, COL);
    Printf_Board(board, ROW, COL);

    win = CheckWin(board, ROW, COL);//如果玩家胜利return X,如果电脑胜利rerturn 0,如果平局return P,如果比赛继续return ' '
    if (win != ' ') {
      break;
    }
    RobotMove(board, ROW, COL);
    Printf_Board(board, ROW, COL);
    win = CheckWin(board, ROW, COL);
    if (win != ' ') {
      break;
    }
//    printf("\033[2J");
    Printf_Board(board, ROW, COL);
  }

  if (win == 'X') {
    printf("玩家胜利!\n");
  } else if (win == '0') {
    printf("电脑胜利!\n");
  } else if (win == 'P') {
    printf("平局!\n");
  }
  return;
}

void menu(void)
{
  printf("*********************************\n");
  printf("*****  1.play game  0.exit  *****\n");
  printf("*********************************\n");
}

void play_game(void)
{
  srand((unsigned)time(NULL));
  int input = 0;
  do {
    menu();
    printf("请输入你的选择:>");
    scanf("%d", &input);
    switch (input) {
    case 1:
      game();
      break;
    case 0:
      printf("退出游戏\n");
      break;
    default:
      printf("选择错误重新输入\n");
      break;
    }
  } while(input);
}

int main()
{
  play_game();

  return 0;
}

