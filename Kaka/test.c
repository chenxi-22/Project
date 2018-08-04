#include <iostream>
#include <stdio.h>
#include <sys/time.h>

int main()
{
  struct timeval tv;
  int ret = gettimeofday(&tv, NULL);
  if (ret < 0) {
    return 1;
  }
  
  printf("%lu\n", tv.tv_sec);

  return 0;
}


