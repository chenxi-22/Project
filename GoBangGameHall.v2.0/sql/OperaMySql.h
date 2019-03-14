#pragma once

#include "/usr/include/mysql/mysql.h"
#include <stdio.h>
#include "Game.h"

#define MAX_LEN 1024

int InsertSql(Local* l, MYSQL* m);

int SelectSql(Local* l, MYSQL* m);
