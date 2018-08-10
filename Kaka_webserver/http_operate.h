#pragma once

#include <errno.h>
#include <ctype.h>
#include <strings.h>
#include <sys/sendfile.h>
#include "comm.h"

#define ROOT_SIZE 12
#define METHOD_SIZE 12
#define CONTENT_LENGTH_SIZE 32
#define CONNECTION_SIZE 32
#define HOST_SIZE 64
#define PATH_SIZE 128
#define URL_SIZE 128
#define QUERY_SIZE 128
#define COOKIE_SIZE 128
#define BUFF_SIZE 1024

#define HOST "index.html"

typedef struct http_request { // HTTP 请求头
  int64_t sock;
  char root[ROOT_SIZE];
  char method[METHOD_SIZE];
  char url[URL_SIZE];
  char path[PATH_SIZE];
  char query[QUERY_SIZE];
  char content_length[CONTENT_LENGTH_SIZE];
  char connection[CONNECTION_SIZE];
  char cookie[COOKIE_SIZE];
  char host[HOST_SIZE];
} http_request;

int get_line(char* buff, int64_t sock);

void* http_handler(void* arg);

void get_connection(http_request* rq, char* buff);

void get_method(http_request* rq, char* buff);

void get_content_length(http_request* rq, char* buff);

void http_request_init(http_request* rq);

void get_cookie(http_request* rq, char* buff);

void get_host(http_request*rq, char* buff);

int http_header_handler(http_request* rq);

int normal_handler(http_request* rq, int size);

int cgi_handler(http_request* rq);

int is_cgi(http_request* rq, int* cgi);

int is_keep_alive(http_request* rq);

