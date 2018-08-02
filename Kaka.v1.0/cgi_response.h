#pragma once

#include "http_response.h"
#include "util.h"

void CgiResponse(int sock, char* method, char* query, char* path); // Cgi 响应



