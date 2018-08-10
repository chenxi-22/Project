#include <stdio.h>

#include "epoll_operate.h"

#define CONFIG_PATH "default.config"

char config_path[] = CONFIG_PATH;

int main() {
  char port[6];
  char root[10];
  kk_read_config(config_path, port, root);

  int64_t sock = kk_start_up(atoi(port));
  if (sock < 0) {
    return 1;
  }
  set_no_block(sock); // 设置 listen_sock 为非阻塞，支持 ET 模式
  sigpipe_handler();

  ThreadPool tp;
  ThreadPoolInit(&tp);

  int epfd = kk_epoll_init(0);
  int ret = kk_epoll_add(epfd, sock, EPOLLIN | EPOLLET);
  if (ret < 0) {
    return 1;
  }

  struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAX_EVENTS_SIZE);
  if (events == NULL) {
    return 1;
  }

  while (1) {
    int ready_nums = kk_epoll_wait(epfd, events, MAX_EVENTS_SIZE, -1);

    ready_events_handler(epfd, events, sock, ready_nums, &tp, root);
  }

  return 0;
}

