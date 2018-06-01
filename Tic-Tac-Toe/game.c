#include "game.h"

void Printf_Board(char board[ROW][COL], int row, int col)//打印棋盘
{
  int i = 0;
  int j = 0;
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      printf(" %c ",board[i][j]);
      if (j < col - 1) {
        printf("|");   
      } 
    }
    printf("\n");
    if (i < row - 1) {
      printf("---|---|---\n");
    }
  }
  printf("\n");
  return;
}

void InitBoard(char board[ROW][COL], int row, int col)//初始化棋盘
{
  int i = 0;
  int j = 0;
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      board[i][j] = ' ';
    }
  }
  return;
}

void PlayerMove(char board[ROW][COL], int row, int col)//玩家走
{
  int x = 0;
  int y = 0;
  printf("玩家走\n");
  while (1) {
    printf("请输入座标:>");
    scanf("%d%d", &x, &y);
    if (board[x - 1][y - 1] == ' ' && x >= 1 && 
        x <= row && y >= 1 && y <= col) {
      board[x - 1][y - 1] = 'X';
      break;
    }
    else
      printf("坐标错误，请重新输入\n");
  }
  return;
}

void RobotMove(char board[ROW][COL], int row, int col) //电脑走
{
  int x = 0;
  int y = 0;

  while (1) {
    x = rand() % row;
    y = rand() % col;
    if (board[x][y] == ' ') {
      board[x][y] = '0';
      return;
    }
  }
}

int IsFull(char board[ROW][COL], int row, int col)//判断棋盘是否满
{
  int i = 0;
  int j = 0;
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      if (board[i][j] == ' ') {
        return 0;
      }
    }
  }
  return 1;
}

char CheckWin(char board[ROW][COL], int row, int col)//判断输赢
{
  //判断胜利的条件：行、列、两个对角线
  //1.行
  int i = 0;
  for (i = 0; i < row; i++) {
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
      return board[i][1];
    }
  }

  //2.列
  for (i = 0; i < col; i++) {
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
      return board[1][i];
    }
  }

  //3.对角线
  if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
    return board[1][1];
  } else if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
    return board[1][1];
  } else if (IsFull(board, ROW, COL) == 1) {
    return 'P';
  } else {
    return ' ';
  }
}
