#include "comm.h"

int64_t kk_start_up(int port) {
  int64_t sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port = htons(port);
  socklen_t len = sizeof(local);

  int opt = 1;
  int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (ret < 0) {
    perror("setsockopt");
    return -1;
  }

  if (bind(sock, (struct sockaddr*)&local, len) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(sock, 5) < 0) {
    perror("listen");
    return -1;
  }

  return sock;
}

int kk_read_config(char* config_path, char* port, char* root) {
  if (port == NULL || root == NULL) {
    return -1;
  }

  char buff[64];
  int fd = open(config_path, O_RDONLY);
  if (fd < 0) {
    return -1;
  }

  ssize_t s = read(fd, buff, sizeof(buff));
  if (s < 0) {
    return -1;
  }
  
  int i = 0;
  for (; i < s; ++i) {
    if (buff[i] == '\n') {
      buff[i] = '\0';
    }
  }
  
  strcpy(port, buff + 5);
  strcpy(root, buff + 15);

  return 0;
}

int set_no_block(int fd)
{
  int flag = fcntl(fd, F_GETFL);
  flag |= O_NONBLOCK;
  int ret = fcntl(fd, F_SETFL, flag);
  return ret;
}

int accepet_handler(int64_t fd)
{
  struct sockaddr_in client;
  socklen_t len = sizeof(client);
  int64_t sock = accept(fd, (struct sockaddr*)&client, &len);
  if (sock < 0) {
    perror("accept");
    return -1;
  }
  set_no_block(sock);

  return sock;
}

int sigpipe_handler()
{
  struct sigaction act;
  act.sa_handler = SIG_IGN;
  int ret = sigaction(SIGPIPE, &act, NULL);
  if (ret < 0) {
    perror("sigaction");
    return -1;
  }

  return 0;
}

