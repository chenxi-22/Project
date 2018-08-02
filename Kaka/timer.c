#include "timer.h"

Timer* TimeInit() // 初始化
{
  Timer* tm = (Timer*)malloc(sizeof(Timer)); 
  tm->head = (TimerNode*)malloc(sizeof(TimerNode));
  tm->head->left_over = 0;
  tm->head->total_t = 0;
  tm->head->prev = tm->head;
  tm->head->next = tm->head;
  tm->t = TIMEOUT;
  return tm;
}

