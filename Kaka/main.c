#include "Comm.h"

struct epoll_event* events;

#define DEFAULT_CONFIG "Default.config"
#define CONFIGSIZE 64

#define PORT 16
#define ROOT 16

char config_path[] = DEFAULT_CONFIG;

// main
int main()
{
  DupErrorDesr();
  
  char buf[CONFIGSIZE];

  int s = ReadConfig(config_path, buf); 
  if (s < 0) {
    return 1;
  }
  char port[PORT];
  char root[ROOT];

  strcpy(port, buf);
  strcpy(root, buf + strlen(port) + 1);

  strcpy(port, port + 5);
  strcpy(root, root + 5);

  int listen_sock = StartUp(atoi(port));
  if (listen_sock < 0) {
    perror("StartUp");
    return 2;
  }

  int epfd = EpollCreate(0);
  if (epfd < 0) {
    perror("EpollCreate");
    return 4;
  }

  int ret = EpollAdd(epfd, listen_sock, EPOLLIN);
  if (ret < 0) {
    perror("EpollAdd");
    return 5;
  }

  events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MAXEVENTS);

  ThreaddPool tp;
  ThreadPoolInit(&tp);

  while (1) {
    int events_num = EpollWait(epfd, events, MAXEVENTS, -1);
    if (events_num < 0) {
      return 6;
    }

    EventsHandler(epfd, events_num, events, listen_sock, &tp, root);
  }
  ThreadPoolDestroy(&tp);

  return 0;
}

