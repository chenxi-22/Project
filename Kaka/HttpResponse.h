#pragma once

#include "Comm.h"
#include "EchoErrorLog.h"
#include "CgiResponse.h"
#include "Util.h"
#include "NormalResponse.h"

#define MAX 1024
#define HOST "index.html"

void* HandlerRequest(void* arg); // 处理请求

