#include "priority_queue.h"

void priority_queue_init(PriorityQueue* pq, int epfd)
{
  if (pq == NULL) 
    return;

  pq->capacity = PRIORITY_QUEUE_SIZE;
  pq->size = 0;
  pq->epfd = epfd;
  TimerNode** tmp = (TimerNode**)malloc(sizeof(TimerNode*) * (pq->capacity));

  int i = 0;
  for (; i < pq->capacity; ++i) {
    tmp[i] = (TimerNode*)malloc(sizeof(TimerNode));
    tmp[i]->is_delete = 1;
    tmp[i]->key_msec = 0;
  }
  pq->tm = tmp;

  return;
}

void priority_queue_destroy(PriorityQueue* pq) 
{
  if (pq == NULL) 
    return;

  pq->size = 0;
  int i = 0;
  for (; i < pq->capacity; ++i) {
    free(pq->tm[i]);
  }
  pq->capacity =  0;

  return;
}

TimerNode* get_min_timer(PriorityQueue* pq)
{
  if (pq == NULL) 
    return NULL;
  return pq->tm[0];
}

int is_empty(PriorityQueue* pq)
{
  if (pq == NULL) 
    return -1;

  return pq->size == 0 ? 1 : 0;
}

void swap(TimerNode** a, TimerNode** b)
{
  TimerNode* tmp = *a;
  *a = *b;
  *b = tmp;
}

void adjust_down(TimerNode** tm, int parent, int size)
{
  int child = parent * 2 + 1;
  while (child < size) {
    if (child + 1 < size && tm[child]->key_msec > tm[child + 1]->key_msec) {
      child += 1;
    }
    if (tm[parent]->key_msec > tm[child]->key_msec) {
      swap(&tm[parent], &tm[child]);
    } else {
      break;
    }
  }
  return;
}

void timer_heap_pop(PriorityQueue* pq)
{
  if (pq == NULL) 
    return;

  swap(&pq->tm[0], &pq->tm[pq->size - 1]);
  adjust_down(pq->tm, 0, pq->size - 1);
  --pq->size;
}

void timer_heap_create(PriorityQueue* pq)
{
  if (pq == NULL) 
    return;

  int parent = pq->size / 2 - 1;
  for (; parent >= 0; --parent) {
    adjust_down(pq->tm, parent, pq->size);
  }
}

void priority_queue_resize(PriorityQueue* pq)
{
  if (pq == NULL) 
    return;

  if (pq->size < pq->capacity) 
    return;

  int new_capacity = pq->capacity * 2 + 1;
  TimerNode** tmp = (TimerNode**)malloc(sizeof(TimerNode*) * new_capacity);
  TimerNode** cur = pq->tm;
  int i = 0;
  for (; i < pq->capacity; ++i) {
    tmp[i]->is_delete = cur[i]->is_delete;
    tmp[i]->key_msec = cur[i]->key_msec;
  }

  for (i = 0; i < pq->capacity; ++i) {
    free(cur[i]);
  }
  pq->tm = tmp;
  pq->capacity = new_capacity;

  return;
}

time_t update_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000; // ms
}

TimerNode* timer_add(PriorityQueue* pq, int sock)
{
  if (pq == NULL) 
    return NULL;

  if (pq->size >= pq->capacity) 
    priority_queue_resize(pq);

  pq->tm[pq->size]->is_delete = 0;
  pq->tm[pq->size]->key_msec = update_time() + TIMEOUT;
  pq->tm[pq->size]->sock = sock;
  ++pq->size;

  return pq->tm[pq->size - 1];
}

void handler(PriorityQueue* pq, TimerNode* min)
{
  if (pq == NULL) 
    return;

  int sock = min->sock;
  min->is_delete = 1;
  // kk_epoll_del(pq->epfd, sock, EPOLLIN | EPOLLET | EPOLLONESHOT);
  epoll_ctl(pq->epfd, EPOLL_CTL_DEL, sock, NULL);
  close(sock);

  return;
}

void timer_del(TimerNode* pos)
{
  if (pos == NULL) 
    return;

  pos->is_delete = 1;
}

void timeout_handler(PriorityQueue* pq)
{
  if (pq == NULL) 
    return;

  time_t now = update_time();
  while (!is_empty(pq)) {
    TimerNode* min = get_min_timer(pq);
    if (min->is_delete == 1) {
      timer_heap_pop(pq);
      continue;
    }

    if (min->key_msec < now) 
      return;

    timer_heap_pop(pq);
    handler(pq, min);
  }
}
