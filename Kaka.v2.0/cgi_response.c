#include "cgi_response.h"

// CgiResponse
void CgiResponse(int sock, char* method, char* query, char* path)
{
  if (strcasecmp(method, "POST") == 0) {
    // POST 方式，此时参数在请求正文里
    int length = atoi(GetLength(sock));
    if (length <= 0) { // 如果 length 小于 0 ，那么意味着此次报头出错，后续不必处理直接 return
      return;
    }

    ClearHead(sock); // 读取报头信息直到空行，此时 sock 里的内容只剩下正文，而 POST 的参数就在正文内

    char buf[MAX/32] =  {0};
    char c = '\0';
    int i = 0;
    while (i < MAX / 32 && length > 0) {
      recv(sock, &c, 1, 0);
      buf[i++] = c;
      length--;
    }
    query = buf;
  } else { // 如果是 GET 方法，那么只需要将 sock 内的数据读完，再进行响应
    ClearHead(sock);
  } // 此时 query 内就是参数

  // 处理 query 里的文件
  query = query + 5;

  char sendbuf[MAX] = {0};
  sprintf(sendbuf, "HTTP/1.0 200 OK\r\n"); 
  send(sock, sendbuf, strlen(sendbuf), 0); // 发送响应行
  bzero(sendbuf, MAX);

  sprintf(sendbuf, "Server: KAKA-Server-v1.0\r\n");  
  send(sock, sendbuf, strlen(sendbuf), 0); 
  bzero(sendbuf, MAX);

  sprintf(sendbuf, "Content-Type: application/octet-stream\r\n");  
  send(sock, sendbuf, strlen(sendbuf), 0); 
  bzero(sendbuf, MAX);

  sprintf(sendbuf, "Content-Disposition: attachment;filename=%s\r\n", query);
  send(sock, sendbuf, strlen(sendbuf), 0); 
  bzero(sendbuf, MAX);

  sprintf(sendbuf, "\r\n");
  send(sock, sendbuf, strlen(sendbuf), 0); // 发送空行 

  T* t = (T*)malloc(sizeof(T));
  t->sock = sock;
  strcpy(t->buf, query);

  int input[2];
  int output[2];

  pipe(input);
  pipe(output);
  pid_t id = fork();

  if (id == 0) {
    close(input[1]);
    close(output[0]);

    dup2(input[0], 0);

    execl(path, path, NULL);
    exit(1);
  } else if (id > 0) {
    close(input[0]);
    close(output[1]);

    write(input[1], t, sizeof(T));

    waitpid(id, NULL, 0);
    close(input[1]);
    close(output[0]);
  } else {
    return;
  }

  return;
}
