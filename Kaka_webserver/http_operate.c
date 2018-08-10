#include "http_operate.h"

int get_line(char* buff, int64_t sock) 
{
  if (buff == NULL) {
    return -1;
  }
  char c = '\0';
  int i = 0;
  while (c != '\n') {
    ssize_t s = recv(sock, &c, 1, 0);
    if (s <= 0) {
      return 0;
    }
    if (c == '\r') {
      recv(sock, &c, 1, MSG_PEEK);
      if (c != '\n') {
        c = '\n';
      } else {
        recv(sock, &c, 1, 0);
      }
    } 
    buff[i++] = c;
  }
  buff[i] = '\0';
  return i;
}

void get_connection(http_request* rq, char* buff) 
{
  if (rq == NULL) {
    return;
  }
  strcpy(rq->connection, buff + 11);

  while (rq->connection[0] == ' ') {
    strcpy(rq->connection, rq->connection + 1);
  }

  int i = 0;
  while (1) {
    if (rq->connection[i] == '\n') {
      rq->connection[i] = '\0';
      break;
    }
    ++i;
  }

  return;
}

void get_method(http_request* rq, char* buff) 
{
  if (rq == NULL) {
    return;
  }

  size_t i = 0;
  size_t j = 0;
  size_t len = strlen(buff);
  while (j < sizeof(rq->method) && i < len && !isspace(buff[i])) {
    rq->method[j++] = buff[i++];
  }
  rq->method[j] = '\0';

  while (isspace(buff[i])) {
    ++i;
  }

  j = 0;
  while (j < sizeof(rq->url) && i < len && !isspace(buff[i])) {
    rq->url[j++] = buff[i++];
  }
  rq->url[j] = '\0';

  j = 0;
  while (i < sizeof(rq->url)) {
    if (rq->url[i] == '?') {
      while (j < sizeof(rq->query) && !isspace(rq->url[i])) {
        rq->query[j++] = rq->url[i++];
      }
      break;
    }
    ++i;
  }
  rq->query[j] = '\0';

  return;
}

void get_content_length(http_request* rq, char* buff) 
{
  if (rq == NULL) {
    return;
  }
  strcpy(rq->content_length, buff + 15);

  while (rq->content_length[0] == ' ') {
    strcpy(rq->content_length, rq->content_length + 1);
  }

  int i = 0;
  while (1) {
    if (rq->content_length[i] == '\n') {
      rq->content_length[i] = '\0';
      break;
    }
    ++i;
  }

  return;
}

void get_cookie(http_request* rq, char* buff) 
{
  if (rq == NULL) {
    return;
  }
  strcpy(rq->cookie, buff + 7);

  while (rq->cookie[0] == ' ') {
    strcpy(rq->cookie, rq->cookie + 1);
  }

  int i = 0;
  while (1) {
    if (rq->cookie[i] == '\n') {
      rq->cookie[i] = '\0';
      break;
    }
    ++i;
  }

  return;
}


void get_host(http_request*rq, char* buff) 
{
  if (rq == NULL) {
    return;
  }
  strcpy(rq->host, buff + 5);

  while (rq->host[0] == ' ') {
    strcpy(rq->host, rq->host + 1);
  }

  int i = 0;
  while (1) {
    if (rq->host[i] == '\n') {
      rq->host[i] = '\0';
      break;
    }
    ++i;
  }

  return;
}

int http_header_handler(http_request* rq) 
{
  int64_t sock = rq->sock;
  char buff[BUFF_SIZE] = {0};
  int handler_flag = 0;
  ssize_t ret = 0;

  while (strcmp(buff, "\n") != 0) {
    ret = get_line(buff, sock);
    if (ret <= 0) {
      return -1;
    }

    if (handler_flag == 0) {
      handler_flag = 1;
      get_method(rq, buff);
      continue;
    }
    if (strstr(buff, "Connection") != NULL) {
      get_connection(rq, buff);
      continue;
    }
    if (strstr(buff, "Content-Length") != NULL) {
      get_content_length(rq, buff);
      continue;
    }
    if (strstr(buff, "Cookie") != NULL) {
      get_cookie(rq, buff);
      continue;
    }
    if (strstr(buff, "Host") != NULL) {
      get_host(rq, buff);
      continue;
    }
  }

  return 0;
}

void http_request_init(http_request* rq)
{
  if (rq == NULL) {
    return;
  }
  bzero(rq, sizeof(http_request));
}

int is_cgi(http_request* rq, int* cgi)
{
  if (rq == NULL) {
    return -1;
  }

  if (strcasecmp(rq->method, "POST") == 0) {
    *cgi = 1;
  } else if (strcasecmp(rq->method, "GET") == 0) {
    if (strlen(rq->query) != 0) {
      *cgi = 1;
    }
  } else { // 该服务器目前只能够处理 GET/POST 两种请求
    return -1;
  }

  sprintf(rq->path, "%s%s", rq->root, rq->url); 
  if (rq->path[strlen(rq->path) - 1] == '/') {
    strcat(rq->path, HOST);
  }

  struct stat st;
  if (stat(rq->path, &st) < 0) {
    perror("stat");
    return -1;
  }
  if (S_ISDIR(st.st_mode)) {
    strcat(rq->path, HOST);
  } else if ((st.st_mode & S_IXGRP) || 
             (st.st_mode & S_IXOTH) ||
             (st.st_mode & S_IXUSR)) {
    *cgi = 1;
  }

  return st.st_size;
}

int is_keep_alive(http_request* rq)
{
  if (rq == NULL) {
    return -1;
  }
  
  if (strcasecmp(rq->connection, "keep-alive") == 0) {
    return 0;
  }

  return -1;
}

