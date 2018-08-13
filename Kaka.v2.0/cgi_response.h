#pragma once

#include "http_response.h"
#include "util.h"

typedef struct T {
  int sock;
  char buf[64];
} T;

void CgiResponse(int sock, char* method, char* query, char* path); // Cgi 响应



