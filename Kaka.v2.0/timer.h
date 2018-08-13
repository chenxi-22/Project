#pragma once

#include "priority_queue.h"
#include "epoll_operations.h"

#define TIMEOUT 500 // 单位 ms

time_t updateTime(); // 更新当前时间

TimerNode* TimerAdd(Priq* pq, int64_t sock); // 添加定时器

void TimerDel(TimerNode* pos); // 删除定时器

void TimerOutHandler(Priq* pq, int epfd); // 超时处理

