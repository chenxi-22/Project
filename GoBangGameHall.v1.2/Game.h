#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <time.h>

#define ROW 10
#define COL 10

typedef struct Coord{
  int x;
  int y;
  int win_sock;
  char flag;
} Coord;

typedef struct Person {
  char id[12];
  char passwd[16];
  struct Person* next;
  // int level; // 天梯积分
} Person;

typedef enum Status {
  READY, // 准备就绪
  PLAY, // 游戏当中
  STAY, // 挂起
  FINISH
} Status; // 定义状态

typedef enum LocalState {
  LOGIN, // 登录
  REGISTE, // 注册
  EXIT, // 退出
  RETURN, // 返回游戏大厅
} LocalState;

typedef struct GameInformation {
  struct Person* p;
  int64_t sock; 
  Status s;
  struct GameInformation* next;
} GameInf; // 游戏内信息

typedef struct Local {
  char id_buf[12];
  char passwd_buf[16];
  LocalState ls;
} Local; // 客户端结构体

typedef struct PKHome { // 房间
  GameInf* p1;
  GameInf* p2;
  struct PKHome* next;
} PKHome;

////////////////////////////////////////////////////////////
// 数据结构与函数分界
////////////////////////////////////////////////////////////

void WelcomeMenu(); // 欢迎界面

void LoginMenu(); // 登录菜单

void Login(Local* l); // 登录

void Exit(Local* l); // 退出

void Registe(Local* l); // 注册

void PlatFormMenu(); // 游戏大厅界面

void GameMenu();


// Game
void InitBoard(char board[ROW][COL], int row, int col);//初始化棋盘
void Printf_Board(char board[ROW][COL], int row, int col);//打印棋盘
void EnterCoord(Coord* c, char board[ROW][COL], int row, int col);//移动
void PlayerOneMove(Coord* c, char board[ROW][COL]);//Server走
void PlayerTwoMove(Coord* c, char board[ROW][COL]);//Client走
char CheckWin(char board[ROW][COL], int row, int col, Coord* c);//判断输赢
void Statement();//声明游戏规则
int Win(char board[ROW][COL], int row, int col, Coord* c);//封装一个判断输赢的函数
void menu();//菜单















