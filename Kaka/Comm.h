#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#include "EpollOperation.h"
#include "EchoErrorLog.h"

typedef struct Arg {
  int64_t sock;
  int epfd;
  char root[32];
} Arg;

int ReadConfig(char* path, char* buf); // 读取默认配置

int Usage(int argc, char* argv[]); // 用法

int StartUp(int port); // 套接字创建, 绑定, 监听

int SetNoBlock(int fd); // 设置为非阻塞

void AcceptHandler(int listen_sock, int epfd); // 连接
