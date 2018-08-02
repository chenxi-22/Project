#include "epoll_operations.h"

int EpollCreate(int flag)
{
  int epfd = epoll_create1(flag);
  if (epfd < 0) 
  {
    EchoErrorLog();
    perror("epoll_create1");
    return -1;
  }

  return epfd;
}

int EpollMod(int epfd, int fd, int events)
{
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
  if (ret < 0) 
  {
    EchoErrorLog();
    perror("epoll_ctl");
    return -1;
  }

  return 0;
}

int EpollAdd(int epfd, int fd, int events)
{
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
  if (ret < 0) 
  {
    EchoErrorLog();
    perror("epoll_ctl");
    return -1;
  }

  return 0;
}

int EpollDel(int epfd, int fd, int events)
{
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
  if (ret < 0) 
  {
    EchoErrorLog();
    perror("epoll_ctl");
    return -1;
  }

  return 0;
}

int EpollWait(int epfd, struct epoll_event* ev, int max_events, int timeout)
{
  int max_ready = epoll_wait(epfd, ev, max_events, timeout);
  if (max_ready < 0) 
  {
    perror("epoll_wait");
    EchoErrorLog();
    return -1;
  }

  return max_ready;
}

void EventsHandler(int epfd, int events_num, struct epoll_event* event, 
                   int listen_sock, ThreaddPool* tp, char root[])
{
  if (event == NULL) 
  {
    return;
  }

  int i = 0;
  for (; i < events_num; ++i) 
  {
    if (event[i].data.fd == listen_sock) 
    {
      AcceptHandler(listen_sock, epfd);
    } else {
      if ((event[i].events & EPOLLERR) || 
          (event[i].events & EPOLLHUP) ||
          !(event[i].events & EPOLLIN)) 
      {
        continue;
      }

      Arg* arg = (Arg *)malloc(sizeof(Arg));
      arg->epfd = epfd;
      arg->sock = event[i].data.fd;
      strcpy(arg->root, root);

      AddWorkQueue(tp, HandlerRequest, (void*)arg);

      DilationThread(tp); 
    }
  } // end for(; i < events_num; ++i)
}



