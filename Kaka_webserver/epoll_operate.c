#include "epoll_operate.h"

int kk_epoll_init(int flag) {
  int epfd = epoll_create1(flag);
  if (epfd < 0) {
    perror("epoll_create1");
    return -1;
  }

  return epfd;
}

int kk_epoll_add(int epfd, int64_t fd, int events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_add");
    return -1;
  }

  return 0;
}

int kk_epoll_mod(int epfd, int64_t fd, int events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event) < 0) {
    perror("epoll_mod");
    return -1;
  }

  return 0;
}

int kk_epoll_del(int epfd, int64_t fd, int events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
    perror("epoll_del");
    return -1;
  }

  return 0;
}

int kk_epoll_wait(int epfd, struct epoll_event* events, int max_events, int timeout) {
  int events_num = epoll_wait(epfd, events, max_events, timeout);
  if (events_num < 0) {
    return -1;
  }

  return events_num;
}

void ready_events_handler(int epfd, struct epoll_event* events, 
                          int64_t listen_sock, int ready_nums, ThreadPool* tp, char* root) {
  if (events == NULL) {
    return;
  }

  int i = 0;
  for (; i < ready_nums; ++i) {
    int fd = events[i].data.fd;
    if (fd == listen_sock) {
      int sock = accepet_handler(fd);
      if (sock < 0) {
        continue;
      }

      // 将获取到的 sock 全部设置成 ET 模式读取
      int ret = kk_epoll_add(epfd, sock, EPOLLIN | EPOLLET | EPOLLONESHOT); 
      if (ret < 0) {
        continue;
      }
    } else if (events[i].events != EPOLLIN) { // 异常就绪，直接关闭
      kk_epoll_del(epfd, fd, EPOLLIN);
      close(fd);
      continue;
    } else {
      // 在添加至任务队列之前，可以先将定时器处理

      arg_set* arg = (arg_set*)malloc(sizeof(arg_set));
      arg->sock = fd;
      arg->epfd = epfd;
      strcpy(arg->root, root);
      AddWorkQueue(tp, http_handler,(void*)arg);
    }
  } // end for(; i < ready_nums; ++i)
}


