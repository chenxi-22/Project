#include "echo_error_log.h"

int DupErrorDesr()
{
  int fd = open(ERRORLOGPATH, O_RDWR | O_APPEND | O_CREAT, 0664);
  if (fd < 0) {
    EchoErrorLog();
    perror("open");
    return -1;
  }

  int ret = dup2(fd, 2);
  if (ret < 0) {
    EchoErrorLog();
    perror("dup2");
    return -1;
  }

  return 0;
}

void EchoErrorLog()
{
  time_t timep;

  struct tm *p;
  time (&timep);
  p=gmtime(&timep);

  char buf[64];
  sprintf(buf, "%2d-%02d-%2d  %02d:%02d:%02d: ", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, 8+p->tm_hour, p->tm_min, p->tm_sec);
  write(2, buf, strlen(buf));

  return;
}

