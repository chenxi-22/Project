#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define PRIORITY_QUEUE 5

typedef struct TimerNode {
  int64_t sock; // sock
  time_t key_msec; // 超时具体时间
  int is_delete;
} TimerNode;

typedef struct PriorityQueue {
  struct TimerNode** tm; // Timer 数组
  int size; // 现在队列内的节点个数
  int capacity;  // 总共的大小
} Priq;

Priq* PriorityQueueInit(); // 初始化

int IsEmpty(Priq* pq); 

TimerNode* GetMinTimerNode(Priq* pq);

void EraseTimerNode(Priq* pq);

void AdjustDown(TimerNode* tm[], int parent, int size);

void TimerNodeHeapCreate(Priq* pq);
