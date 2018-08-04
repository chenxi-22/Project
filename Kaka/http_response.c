#include "http_response.h"

// HandlerRequest
void* HandlerRequest(void* arg)
{
  Arg* cur = (Arg*)arg;

  int epfd = cur->epfd;
  int64_t sock = cur->sock;
  char* root = cur->root;
  TimerManager* tm = cur->tm;
  pthread_mutex_t lock = cur->lock;
  free(cur);

  char buf[MAX] = {0};
  char method[MAX/32] = {0};
  char url[MAX] = {0};
  char path[MAX] = {0};
  char* query = NULL;
  int cgi = 0;

#if 0

  while (strcmp(buf, "\n") != 0) {
    GetLine(sock, buf, sizeof(buf));
    printf("%s",buf);
  }

#else

  int s = GetLine(sock, buf, sizeof(buf));
  if (s < 0) { // 读取一行，此时 buf 里存放的是请求行数据
    close(sock);
  }

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

  sprintf(path, "%s%s", root, url);
  if (path[strlen(path) - 1] == '/') {
    strcat(path, HOST);
  }

  struct stat st;
  if (stat(path, &st) < 0) {
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
    NormalResponse(sock, path, st.st_size);
  }

#endif

  // HandlerError(error_code);
  EpollDel(epfd, sock, EPOLLIN | EPOLLONESHOT);
  pthread_mutex_lock(&lock);
  Timer* pos = TimerFind(tm->head, sock);
  TimerDel(tm->head, pos);
  pthread_mutex_unlock(&lock);
  close(sock);
  return NULL;
}
