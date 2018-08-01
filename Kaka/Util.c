#include "Util.h"

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

// ClearHead
void ClearHead(int sock)
{
  char buf[MAX] = {0};

  while (strcmp(buf, "\n") != 0) {
    GetLine(sock, buf, sizeof(buf));
  }

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

