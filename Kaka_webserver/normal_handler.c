#include "http_operate.h"

int normal_handler(http_request* rq, int size)
{
  if (rq == NULL) {
    return -1;
  }

  int fd = open(rq->path, O_RDONLY);
  if (fd < 0) {
    return -1;
  }

  char send_buff[BUFF_SIZE] = {0};
  int sock = rq->sock;

  sprintf(send_buff, "HTTP/1.1 200 OK\r\n");
  ssize_t s = send(sock, send_buff, strlen(send_buff), 0);
  if (s <= 0) {
    perror("send1");
    close(fd);
    return -1;
  }

  sprintf(send_buff, "Server: kaka-web-server/1.0\r\n");
  s = send(sock, send_buff, strlen(send_buff), 0);
  if (s <= 0) {
    perror("send2");
    close(fd);
    return -1;
  }

  sprintf(send_buff, "\r\n");
  printf("%s", send_buff);
  s = send(sock, send_buff, strlen(send_buff), 0);
  if (s <= 0) {
    perror("send3");
    close(fd);
    return -1;
  }

  s = sendfile(sock, fd, NULL, size);
  if (s <= 0) {
    perror("sendfile");
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}
