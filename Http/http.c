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

#define MAX 1024
#define HOST "index.html"

// Usage
static int Usage(int argc, char* argv[]) // 用法
{
  if (argc != 2) {
    printf("Usage: %s [port]\n", argv[0]);
    return 1;
  }
  return 0;
}

// StartUp
static int StartUp(int port) // 创建套接字
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
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
    perror("bind");
    return -1;
  }

  if (listen(sock, 5) < 0) {
    perror("listen");
    return -1;
  }

  return sock;
}

// GetLine
int GetLine(int sock, char line[], int size) // 按行读取
{
  char c = '\0';
  int i = 0;
  ssize_t s = 0;
  while (i < size && c != '\n') {
    s = recv(sock, &c, 1, 0);
    if (s < 0) {
      break;
    }

    // 根据 HTTP 协议，按行处理，换行的可能有三种：
    // a) \n 如果是 \n ，那么不需要做特殊处理
    // b) \r
    // c) \r\n
    // 如果是 \r ，需要判断下一个字符是什么：
    // 1) 如果是 \n ，那么只需要再读一次，将第二次读到的 \n 放入 line 即可
    // 2) 如果不是 \n ，那么直接在 line 内放入 \n 即可
    // 这里判断下一个字符采用的是窥探的方式
    // 就是将 recv 函数里的 flag 选项设置为 MSG_PEEK 即可
    // 只拷贝 sock 缓冲区内部的数据值，不取出

    if (c == '\r') {
      recv(sock, &c, 1, MSG_PEEK);
      if (c != '\n') {
        c = '\n';
      } else {
        recv(sock, &c, 1, 0);
      }
    }
    line[i++] = c;
  }
  line[i] = '\0';
  return i;
}

// HandlerError
static void HandlerError(int error_code)
{
  // printf("error -> %d\n",error_code);

  return;
}

// ClearHead
static void ClearHead(int sock)
{
  char buf[MAX] = {0};

  while (strcmp(buf, "\n") != 0) {
    GetLine(sock, buf, sizeof(buf));
  }

  return;
}

// NormalResponse
static void NormalResponse(int sock, char* path, int size)
{
  int pathfd = open(path, O_RDONLY);
  if (pathfd < 0) {
    return;
  }

  char sendbuf[MAX] = {0};

  ClearHead(sock);
  sprintf(sendbuf, "HTTP/1.0 200 OK\r\n"); 
  send(sock, sendbuf, strlen(sendbuf), 0); // 发送响应行

 //  sprintf(sendbuf, "Content-Type: text/html;charset=UTF-8\r\n");
 //  send(sock, &sendbuf, sizeof(sendbuf), 0); // 发送响应报头

  sprintf(sendbuf, "\r\n");
  send(sock, sendbuf, strlen(sendbuf), 0); // 发送空行 

  sendfile(sock, pathfd, NULL, size); // 将要客户端请求的资源发送过去

  close(pathfd);
  return;
}

// GetLength
char* GetLength(int sock)
{
  char buf[MAX] = {0};
  char* length = NULL;
  while (strcmp(buf,"\n") != 0) {
    GetLine(sock, buf, sizeof(buf));
    if (strstr(buf, "Content-Length:") != NULL) {
      break;
    }
  }
  length = buf+strlen("Content-Length:");
  return length;
}

// CgiResponse
static void CgiResponse(int sock, char* method, char* query, char* path)
{
  if (strcasecmp(method, "POST") == 0) {
    // POST 方式，此时参数在请求正文里
    int length = atoi(GetLength(sock));

    ClearHead(sock);

    char buf[MAX/32] =  {0};
    char c = 'a';
    int i = 0;
    while (length--) {
      recv(sock, &c, 1, 0);
      buf[i++] = c;
    }
    query = buf;
  } // 到达此处的时候 query 里存放的就是正规参数
}

// HandlerRequest
static void* HandlerRequest(void* arg)
{
  int sock = (int)(int64_t)arg;
  char buf[MAX] = {0};
  char method[MAX/32] = {0};
  char url[MAX] = {0};
  char path[MAX] = {0};
  char* query = NULL;
  int error_code;
  int cgi = 0;

#if 0
  while (strcmp(buf, "\n") != 0) {
    GetLine(sock, buf, sizeof(buf));
    printf("%s",buf);
  }
#else
  int s = GetLine(sock, buf, sizeof(buf));
  if (s < 0) { // 读取一行，此时 buf 里存放的是请求行数据
    error_code = 1;
    HandlerError(error_code);
    close(sock);
  }
  printf("%d\n",s);
  printf("========请求行========\n%s\n",buf);

  size_t i = 0;
  size_t j = 0;

  while (i < sizeof(method) - 1 && j < sizeof(buf) && !isspace(buf[j])) { // 在 buf 里读取请求方式 method 
    method[i++] = buf[j++];
  }
  method[i] = '\0';

  if (strcasecmp(method, "GET") == 0) { 
    // 如果请求方式为 GET 那么要判断它的 url 内是否带有参数：
    // 1) 如果带参数，采用 cgi ，cgi = 1;
    // 2) 如果没有参数，不采用 cgi，cgi = 0;
  } else if (strcasecmp(method, "POST") == 0) { // 如果请求方式为 POST 那么一定采用 cgi 
    cgi = 1;
  } else { // 这里只处理 GET 与 POST 两种请求方法
    error_code = 2;
    HandlerError(error_code);
    close(sock);
  }

  while (j < sizeof(buf) && isspace(buf[j])) { // 防止 method 与 url 之间有很多空格
    j++;
  }

  i = 0;

  while (i < sizeof(url) - 1 && j < sizeof(buf) && !isspace(buf[j])) { // 在 buf 里读取请求资源 url 
    url[i++] = buf[j++];
  }
  url[i] = '\0';

  if (strcasecmp(method, "GET") == 0) { // 处理请求方式为 GET 的时候 url 里是否有参数
    query = url;
    while (*query != '\0') {
      if (*query == '?') {
        *query = '\0';
        query++;
        cgi = 1;
        break;
      }
      query++;
    }
  } // end if (strcasecmp(method, "GET") == 0)

  sprintf(path, "wwwroot%s", url);
  if (path[strlen(path) - 1] == '/') {
    strcat(path, HOST);
  }

  struct stat st;
  if (stat(path, &st) < 0) {
    error_code = 3;
    HandlerError(error_code);
    close(sock);
  } else {
    if (S_ISDIR(st.st_mode)) {
      strcat(path, HOST);
    } else if((st.st_mode & S_IXOTH) || (st.st_mode & S_IXGRP) \
              || (st.st_mode & S_IXUSR)) {
      cgi = 1;
    }
  }

  if (cgi == 1) { // cgi 模式运行
    CgiResponse(sock, method, query, path);
  } else { // 正常模式运行 
    printf("method:%s   path:%s\n",method, path);
    NormalResponse(sock, path, st.st_size);
  }

#endif

  // HandlerError(error_code);
  close(sock);

  return NULL;
}

// main
int main(int argc, char* argv[])
{
  int ret = Usage(argc, argv);
  if (ret != 0) {
    return ret;
  }

  int listen_sock = StartUp(atoi(argv[1])); // 创建监听套接字 listen_sock 
  if (listen_sock < 0) {
    perror("StartUp");
    return 2;
  }

  while (1) {
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int64_t sock = accept(listen_sock, (struct sockaddr*)&client, &len); // 获取连接
    if (sock < 0) {
      perror("accept");
      return 3;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, HandlerRequest, (void *)sock);
    pthread_detach(tid); // 线程分离
  }

  return 0;
}


