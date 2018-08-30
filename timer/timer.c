#include "timer.h"

time_t update_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000; // ms
}

TimerNode* timer_add(PriorityQueue* pq, int sock)
{
  if (pq == NULL) {
    return NULL;
  }

  if (pq->size >= pq->capacity) {
    priority_queue_resize(pq);
  }
  pq->tm[pq->size]->is_delete = 0;
  pq->tm[pq->size]->key_msec = update_time() + TIMEOUT;
  pq->tm[pq->size]->sock = sock;
  ++pq->size;

  return pq->tm[pq->size];
}

void handler(PriorityQueue* pq, TimerNode* min)
{
  if (pq == NULL) {
    return;
  }
  int sock = min->sock;
  min->is_delete = 1;
  epoll_ctl(pq->epfd, EPOLL_CTL_DEL, sock, NULL);
  close(sock);

  return;
}

void timer_del(TimerNode* pos)
{
  if (pos == NULL) {
    return;
  }
  pos->is_delete = 1;
}

void timeout_handler(PriorityQueue* pq)
{
  if (pq == NULL) {
    return;
  }

  time_t now = update_time();
  while (!is_empty(pq)) {
    TimerNode* min = get_min_timer(pq);
    if (min->is_delete == 1) {
      timer_heap_pop(pq);
      continue;
    }
    if (min->key_msec < now) {
      return;
    }
    timer_heap_pop(pq);
    handler(pq, min);
  }
}
