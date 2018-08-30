#pragma once

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <unistd.h>

// #include "epoll_operate.h"

#define TIMEOUT 500 // ms

#define PRIORITY_QUEUE_SIZE 10 // 就绪队列初始长度

typedef struct TimerNode {
  time_t key_msec; // 超时时间
  int is_delete; // 是否有效
  int sock;
} TimerNode;

typedef struct PriorityQueue {
  struct TimerNode** tm;
  int size;
  int capacity;
  int epfd;
} PriorityQueue;

void priority_queue_init(PriorityQueue* pq, int epfd);

void priority_queue_destroy(PriorityQueue* pq);

TimerNode* get_min_timer(PriorityQueue* pq);

int is_empty(PriorityQueue* pq);

void timer_heap_create(PriorityQueue* pq);

void timer_heap_pop(PriorityQueue* pq);

void priority_queue_resize(PriorityQueue* pq);
