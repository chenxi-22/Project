#pragma once

#include "priority_queue.h"

time_t update_time();                              // 更新当前时间

TimerNode* timer_add(PriorityQueue* pq, int sock); // 添加 timer 节点

void timer_del(TimerNode* pos);                    // 惰性删除 timer 节点

void timeout_handler(PriorityQueue* pq);           // 超时处理函数

