#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef int Time_t;

typedef struct TimerNode {
  Time_t total_t;
  Time_t left_over;
  struct TimerNode* next;
  struct TimerNode* prev;
} TimerNode;

typedef struct Timer {
  Time_t t;
  struct TimerNode* head;
} Timer;

#define TIMEOUT 5

void* TimeOperate(void* arg);

Timer* TimeInit(); // 初始化

Timer* TimeCheck(); // 检测

