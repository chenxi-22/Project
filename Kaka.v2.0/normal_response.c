#include "normal_response.h"

// NormalResponse
void NormalResponse(int sock, char* path, int size)
{
  int pathfd = open(path, O_RDONLY);
  if (pathfd < 0) {
    return;
  }

  char sendbuf[MAX] = {0};

  ClearHead(sock);
  sprintf(sendbuf, "HTTP/1.0 200 OK\r\n"); 
  int ret = send(sock, sendbuf, strlen(sendbuf), 0); // 发送响应行
  if (ret <= 0) {
    perror("send");
    goto close;
  }

 //  sprintf(sendbuf, "Content-Type: text/html;charset=UTF-8\r\n");
 //  send(sock, &sendbuf, sizeof(sendbuf), 0); // 发送响应报头

  sprintf(sendbuf, "\r\n");
  ret = send(sock, sendbuf, strlen(sendbuf), 0); // 发送空行 
  if (ret <= 0) {
    perror("send");
    goto close;
  }

  ret = sendfile(sock, pathfd, NULL, size); // 将要客户端请求的资源发送过去
  if (ret <= 0) {
    perror("sendfile");
    goto close;
  }

close:
  close(pathfd);
  return;
}

