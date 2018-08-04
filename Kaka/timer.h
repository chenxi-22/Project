#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define TIMEOUT 5
#define INIT_TIMER 5

typedef struct Timer{
  int64_t sock; // sock
  time_t _init_time; // 初始化计时器时间
  time_t _flag; // 标记, 如果 flag == 1 意味着该节点被占用, 如果 flag == 0 代表该节点可复用
  struct Timer* _next; // 尾指针
  struct Timer* _prev; // 头指针
} Timer;

typedef struct TimerManager {
  time_t timeout;
  time_t present_time;
  int num;
  struct Timer* head;
} TimerManager;

typedef Timer* (*Handler)(int64_t); // 定义函数指针

time_t GetPresentTime(); // 获取当前时间

TimerManager* TimerManagerInit(); // 初始化时间管理器

void TimerDel(Timer* head, Timer* pos); // 删除 Timer

void TimerManagerDestroy(TimerManager* tm); // 销毁时间管理器

void* TimeOutHandler(void* arg); // 处理超时

void TimerPush(Timer* head, int64_t sock); // 添加至定时器队列

Timer* TimerFind(Timer* head, int64_t sock); // 在队列中查找
