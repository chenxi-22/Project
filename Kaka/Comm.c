#include "Comm.h"

int ReadConfig(char* path, char buf[])
{
  if (path == NULL) {
    return -1;
  }

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  char c = '\0';
  int i = 0;
  int flag = 0;

  while (1) {
    int s = read(fd, &c, 1);
    if (s < 0) {
      break;
    }
    if (c == '\n') {
      c = '\0';
      flag++;
      if (flag == 2) {
        break;
      }
    }
    buf[i++] = c;
  }
  return i;
}

// Usage
int Usage(int argc, char* argv[]) // 用法
{
  char buf[] = "Usage Error!\n";
  if (argc != 2) {
    printf("Usage: %s [port]\n", argv[0]);
    EchoErrorLog();
    write(2, buf, strlen(buf));
    return -1;
  }
  return 0;
}

// StartUp
int StartUp(int port) // 创建套接字
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    EchoErrorLog();
    perror("socket");
    return -1;
  }

  int opt = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 设置端口号可复用

  struct sockaddr_in local;
  socklen_t len = sizeof(local);
  bzero(&local, len);
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr*)&local, len) < 0) {
    EchoErrorLog();
    perror("bind");
    return -1;
  }

  if (listen(sock, 5) < 0) {
    EchoErrorLog();
    perror("listen");
    return -1;
  }

  return sock;
}

int SetNoBlock(int fd) // 设置为非阻塞
{
  int flag = fcntl(fd, F_GETFL, 0);
  flag |= O_NONBLOCK;
  int ret = fcntl(fd, F_SETFL, flag);
  if (ret < 0) {
    perror("fcntl");
    return -1;
  }

  return 0;
}

void AcceptHandler(int listen_sock, int epfd)
{
  struct sockaddr_in client;
  socklen_t len = sizeof(client);

  int64_t sock = accept(listen_sock, (struct sockaddr*)&client, &len);
  if (sock < 0) {
    perror("accept");
    return;
  }

//  int ret = SetNoBlock(sock);
//  if (ret < 0) {
//    return;
//  }

  int ret = EpollAdd(epfd, sock, EPOLLIN | EPOLLET);
  if (ret < 0) {
    return;
  }
}
