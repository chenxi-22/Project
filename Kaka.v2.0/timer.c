#include "timer.h"

time_t updateTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t ud_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return ud_time;
}

void TimerResize(Priq* pq) // 扩容
{
  if (pq == NULL) {
    return;
  }

  int capacity = pq->capacity;
  int new_capacity = capacity * 2 + 1;
  TimerNode** tmp = (TimerNode**)malloc(sizeof(TimerNode*) * new_capacity);

  int i = 0;
  for (; i < capacity; ++i) {
    tmp[i] = (TimerNode*)malloc(sizeof(TimerNode));
    tmp[i]->key_msec = pq->tm[i]->key_msec;
    tmp[i]->sock = pq->tm[i]->sock;
    tmp[i]->is_delete = pq->tm[i]->is_delete;
  }
  for (i = 0; i < capacity; ++i) {
    free(pq->tm[i]);
  }
  pq->capacity = new_capacity;
  pq->tm = tmp;
  return;
}

TimerNode* TimerAdd(Priq* pq, int64_t sock) // 添加定时器
{
  if (pq == NULL) {
    return NULL;
  }

  int capacity = pq->capacity; 
  int size = pq->size;
  if (size >= capacity) {
    TimerResize(pq);
  }

  pq->tm[size] = (TimerNode*)malloc(sizeof(TimerNode));
  pq->tm[size]->key_msec = updateTime();
  pq->tm[size]->sock = sock;
  pq->tm[size]->is_delete = 0;
  ++pq->size;
  TimerNodeHeapCreate(pq);

  return pq->tm[pq->size - 1];
}

void TimerDel(TimerNode* pos) // 删除定时器
{
  if (pos == NULL) {
    return;
  }

  pos->is_delete = 1;
  return;
}

void Handler(TimerNode* min, int epfd)
{
 // perror("handler");
 // EpollDel(epfd, min->sock, EPOLLIN);
  close(min->sock);
}

void TimerOutHandler(Priq* pq, int epfd)
{
  if (pq == NULL) {
    return;
  }
  time_t now_time = updateTime();
  
  while (!IsEmpty(pq)){
    TimerNode* min = GetMinTimerNode(pq);
    if (min->is_delete == 1) {
      EraseTimerNode(pq);
      continue;
    }
    if (min->key_msec > now_time) {
      break;
    }
    EraseTimerNode(pq);
    Handler(min, epfd);
  }

  return;
}

