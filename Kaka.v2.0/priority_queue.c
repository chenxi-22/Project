#include "priority_queue.h"

Priq* PriorityQueueInit() // 初始化
{
  Priq* res = (Priq*)malloc(sizeof(Priq));
  if (res == NULL) {
    perror("malloc");
    return NULL;
  }

  res->tm = NULL;
  res->capacity = 0;
  res->size = 0;

  return res;
}

int IsEmpty(Priq* pq) 
{
  if (pq == NULL) {
    return -1;
  }

  return pq->size == 0 ? 1 : 0;
}

TimerNode* GetMinTimerNode(Priq* pq)
{
  if (pq == NULL) {
    return  NULL;
  }
  
  return pq->tm[0];
}

void swap(TimerNode** a, TimerNode** b)
{
  TimerNode* tmp = *a;
  *a = *b;
  *b = tmp;
}

void AdjustDown(TimerNode* tm[], int parent, int size) // 下沉
{
  if (tm == NULL) {
    return;
  }

  int child = 2 * parent + 1;
  while (child < size) {
    if (child + 1 < size && tm[child]->key_msec > tm[child + 1]->key_msec) {
      child = child + 1;
    }
    if (tm[child]->key_msec < tm[parent]->key_msec) { 
      swap(&tm[child], &tm[parent]);
      parent = child;
      child = 2 * parent + 1;
    } else {
      break;
    }
  }

  return;
}

void TimerNodeHeapCreate(Priq* pq)
{
  if (pq == NULL || pq->size == 0) {
    return;
  }

  int parent = pq->size / 2 - 1;
  while (parent >= 0) {
    AdjustDown(pq->tm, parent, pq->size);
    --parent;
  }

  return;
}

void EraseTimerNode(Priq* pq)
{
  if (pq == NULL || pq->size == 0) {
    return;
  }

  TimerNode* top = pq->tm[0];
  TimerNode* last = pq->tm[pq->size - 1];
  swap(&top, &last);
  AdjustDown(pq->tm, 0, pq->size - 1);
  --pq->size;

  return;
}


