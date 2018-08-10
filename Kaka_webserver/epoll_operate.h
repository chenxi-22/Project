#pragma once

#include <stdio.h>
#include <sys/epoll.h>

#include "thread_pool.h"
#include "comm.h"
#include "http_operate.h"

typedef struct arg_set {
  int64_t sock;
  int epfd;
  char root[ROOT_SIZE];
} arg_set;

#define MAX_EVENTS_SIZE 1024

int kk_epoll_init(int flag); // 这里用 epoll_create1 来创建 epoll 句柄

int kk_epoll_add(int epfd, int64_t fd, int events);

int kk_epoll_mod(int epfd, int64_t fd, int events);

int kk_epoll_del(int epfd, int64_t fd, int events);

int kk_epoll_wait(int epfd, struct epoll_event* events, int max_events, int timeout);

void ready_events_handler(int epfd, struct epoll_event* events,
                          int64_t listen_sock, int ready_nums, ThreadPool* tp, char* root);
