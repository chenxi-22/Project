#pragma once

#include <sys/epoll.h>
#include <stdio.h>
#include <errno.h>
#include "ThreadPool.h"
#include "HttpResponse.h"

#define MAXEVENTS 1024

int EpollCreate(int flag);

int EpollMod(int epfd, int fd, int events);

int EpollAdd(int epfd, int fd, int events);

int EpollDel(int epfd, int fd, int events);

int EpollWait(int epfd, struct epoll_event* ev, int max_events, int timeout);

void EventsHandler(int epfd, int events_num, struct epoll_event* event, 
                   int listen_sock, ThreaddPool* tp, char root[]);

