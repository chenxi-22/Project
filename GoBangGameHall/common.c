#include "common.h"

// Usage
int Usage(int argc, char* argv[]) 
{
  if (argc < 2) {
    printf("Usage: %s [port]",argv[0]);
    return 0;
  }

  return 1;
}

int ServerStartUp(int port) // StartUp 监听套接字创建 绑定等
{
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sock < 0) {
    perror("socket");
    exit(2);
  }

  int opt = 1;
  setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in local;
  bzero(&local, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port = htons(port);

  if (bind(listen_sock, (struct sockaddr*)&local, sizeof(local)) < 0) {
    perror("bind");
    exit(3);
  }
  
  if (listen(listen_sock, 5) < 0) {
    perror("listen");
    exit(4);
  }

  return listen_sock;
}

int ClientStartUp(int port) // StartUp 监听套接字创建 绑定等
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(2);
  }

  struct sockaddr_in local;
  bzero(&local, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port = htons(port);
  socklen_t len = sizeof(local);

  if (connect(sock, (struct sockaddr*)&local, len) < 0) {
    perror("connect");
    exit(3);
  }

  return sock;
}


int64_t GetConnect(int listen_sock, struct sockaddr* client, socklen_t len) // 获取连接，得到 sock
{
  int sock = accept(listen_sock, (struct sockaddr*)&client, &len);
  if (sock < 0) {
    perror("accept");
    exit(5);
  }

  return sock;
}
