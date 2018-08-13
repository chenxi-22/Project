#pragma once

#include "comm.h"
#include "echo_error_log.h"
#include "cgi_response.h"
#include "util.h"
#include "normal_response.h"
#include "timer.h"

#define MAX 1024
#define HOST "index.html"

void* HandlerRequest(void* arg); // 处理请求

