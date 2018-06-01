#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROW 3
#define COL 3

void InitBoard(char board[ROW][COL], int row, int col);//初始化棋盘
void Printf_Board(char board[ROW][COL], int row, int col);//打印棋盘
void PlayerMove(char board[ROW][COL], int row, int col);//玩家走
void RobotMove(char board[ROW][COL], int row, int col);//电脑走
char CheckWin(char board[ROW][COL], int row, int col);//判断输赢

