#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>

#include "cgi_response.h"

void sigpipe_handler() {
  struct sigaction act;
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, NULL);
}

int  main()
{
  sigpipe_handler();

  T* buff = (T*)malloc(sizeof(T));
  bzero(buff, sizeof(T));

  read(0, buff, sizeof(T));

  char sendbuf[128] = {0};

  sprintf(sendbuf, "wwwroot/cgi/data/%s", buff->buf);
  printf("%s\n", buff->buf);

  int fd = open(sendbuf, O_RDONLY);
  printf("fd=%d\n", fd);
  struct stat st;
  stat(sendbuf, &st);
  sendfile(buff->sock, fd, NULL, st.st_size);

  close (fd);
  return 0;
}


