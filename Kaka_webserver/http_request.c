#include "http_operate.h"
#include "epoll_operate.h"

void* http_handler(void* cur) {
  arg_set* arg = (arg_set*)cur;
  int64_t sock = arg->sock;
  int epfd = arg->epfd;

  http_request* rq = (http_request*)malloc(sizeof(http_request));
  http_request_init(rq);

  rq->sock = sock;
  strcpy(rq->root, arg->root);
  free(cur);

  int cgi = 0;
  int size = 0;

  int ret = http_header_handler(rq);
  if (ret < 0) {
    goto close;
  }

  size = is_cgi(rq, &cgi);  
  if (size < 0) {
    goto close;
  }

 if (cgi == 0) {
   ret = normal_handler(rq, size);
   if (ret < 0) {
     goto close;
   }
 } else {
   ret = cgi_handler(rq);
   if (ret < 0) {
     goto close;
   }
 }

  ret = is_keep_alive(rq);
  if (ret < 0) {
    goto close;
  } else {
  //  kk_epoll_mod(epfd, rq->sock, EPOLLET | EPOLLIN | EPOLLONESHOT);
  //  return NULL;
    goto close;
  }

close:
  kk_epoll_del(epfd, rq->sock, EPOLLET | EPOLLIN | EPOLLONESHOT);
  printf("after send2\n");
  close(rq->sock);
  free(rq);

  return NULL;
}

