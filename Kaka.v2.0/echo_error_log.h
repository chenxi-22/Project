#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ERRORLOGPATH "Error.log"

int DupErrorDesr(); // 重定向错误日志文件描述符

void EchoErrorLog(); // 打印日志
