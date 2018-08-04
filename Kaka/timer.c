#include "timer.h"

time_t GetPresentTime()
{
  struct timeval tv;

  int ret = gettimeofday(&tv, NULL);
  if (ret < 0) {
    perror("gettimeofday");
    return 0;
  }

  return tv.tv_sec;
}

static Timer* TimerInitCreate(int64_t sock) 
{
  Timer* _tmp = (Timer*)malloc(sizeof(Timer));
  if (_tmp == NULL) {
    return NULL;
  }

  _tmp->sock = sock;
  _tmp->_init_time = GetPresentTime();
  _tmp->_flag = 0;
  _tmp->_next = _tmp;
  _tmp->_prev = _tmp;

  return _tmp;
}

static Timer* TimerCreate(int64_t sock) 
{
  Timer* _tmp = (Timer*)malloc(sizeof(Timer));
  if (_tmp == NULL) {
    return NULL;
  }

  _tmp->sock = sock;
  _tmp->_init_time = GetPresentTime();
  _tmp->_flag = 1;
  _tmp->_next = _tmp;
  _tmp->_prev = _tmp;

  return _tmp;
}

static void TimerAdd(Timer* head, int64_t sock, Handler fun) 
{
  if (head == NULL) {
    return;
  }

  Timer* _new_timer = fun(sock); 
  if (_new_timer == NULL) {
    return;
  }

  _new_timer->_next = head;
  _new_timer->_prev = head->_prev;
  head->_prev->_next = _new_timer;
  head->_prev = _new_timer;

  return;
}

TimerManager* TimerManagerInit() 
{
  TimerManager* _time_manager = (TimerManager*)malloc(sizeof(TimerManager));
  if (_time_manager == NULL) {
    perror("malloc _time_manager error");
    return NULL;
  }

  _time_manager->head = (Timer*)malloc(sizeof(Timer));
  if (_time_manager->head == NULL) {
    perror("malloc head");
    return NULL;
  }

  _time_manager->head->sock = -1;
  _time_manager->head->_init_time = 0;
  _time_manager->head->_flag = 0;
  _time_manager->head->_next = _time_manager->head;
  _time_manager->head->_prev = _time_manager->head;

  _time_manager->num = INIT_TIMER;
  int i = 0;
  while (i < _time_manager->num) {
    TimerAdd(_time_manager->head, 0, TimerInitCreate);
    ++i;
  }
  
  _time_manager->present_time = GetPresentTime();
  _time_manager->timeout = TIMEOUT;

  return _time_manager;
}

void TimerPush(Timer* head, int64_t sock)
{
  if (head == NULL) {
    return;
  }

  Timer* cur = head->_next;
  while (cur != head) {
    if (cur->_flag == 0) {
      cur->_flag = 1;
      cur->sock = sock;
      cur->_init_time = GetPresentTime();
      return;
    }
    cur = cur->_next;
  }
  TimerAdd(head, sock, TimerCreate);
  return;
}

static void TimerDestroy(Timer* pos)
{
  free(pos);
}

void TimerDel(Timer* head, Timer* pos) 
{
  if (head == NULL || head->_next == head 
      || pos == NULL) {
    return;
  }

  pos->_flag = 0;

  return;
}

Timer* TimerFind(Timer* head, int64_t sock) 
{
  if (head == NULL) {
    return NULL;
  }

  Timer* cur = head->_next;
  while (cur != head) {
    if (cur->sock == sock) {
      return cur;
    }
  }
  return NULL;
}

void TimerManagerDestroy(TimerManager* tm)
{
  Timer* cur = tm->head->_next;
  while (cur != tm->head) {
    TimerDel(tm->head, cur);
    cur = cur->_next;
  }
  free(tm);
  return;
}

static int _TimeOutHandler(TimerManager* tm)
{
  if (tm == NULL) {
    return -1;
  }

  int count = 0;
  
  tm->present_time = GetPresentTime();
  Timer* cur = tm->head->_next;
  time_t timeout = tm->present_time + tm->timeout;
  while (cur != tm->head) {
    if (cur->_init_time + tm->timeout > timeout) {
      ++count;
      TimerDel(tm->head, cur);
    }
    cur = cur->_next;
  }
  return count;
}

void* TimeOutHandler(void* arg)
{
  if (arg == NULL) {
    return NULL;
  }

  TimerManager* tm = (TimerManager*)arg;

  while (1) {
    sleep(5);
    _TimeOutHandler(tm);
  }

  return NULL;
}


