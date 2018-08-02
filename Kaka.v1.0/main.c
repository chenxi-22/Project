#include "comm.h"

#define DEFAULT_CONFIG "default.config"
#define CONFIG_SIZE 64

#define PORT 16
#define ROOT 16

char config_path[] = DEFAULT_CONFIG;
struct epoll_event* events;

// main
int main()
{
  DupErrorDesr();
  
  char buf[CONFIG_SIZE];
  char port[PORT];
  char root[ROOT];

  int s = readConfig(config_path, buf, root, port); 
  if (s < 0) {
    return 1;
  }

  int listen_sock = StartUp(atoi(port));
  if (listen_sock < 0) {
    perror("StartUp");
    return 1;
  }
  
  int epfd = EpollCreate(0);
  if (epfd < 0) {
    perror("EpollCreate");
    return 1;
  }

  // 增加 EPOLLONESHOT 是为了保证每一个就绪的文件描述符有且只有一个线程为其服务
  int ret = EpollAdd(epfd, listen_sock, EPOLLIN | EPOLLONESHOT); 
  if (ret < 0) {
    perror("EpollAdd");
    return 1;
  }

  events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAXEVENTS);

  ThreaddPool tp;
  ThreadPoolInit(&tp);

  while (1) {
    int events_num = EpollWait(epfd, events, MAXEVENTS, -1);
    if (events_num < 0) {
      return 1;
    }

    EventsHandler(epfd, events_num, events, listen_sock, &tp, root);
  }

  close(epfd);
  ThreadPoolDestroy(&tp);

  return 0;
}

