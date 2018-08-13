#pragma once

#include "comm.h"

int GetLine(int sock, char line[], int size); // 按行读取

void ClearHead(int sock); // 清空头部

char* GetLength(int sock); // 获取长度


