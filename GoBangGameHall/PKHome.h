#pragma once

#include "Game.h"

void PKHomeInit(PKHome** phead);

PKHome* PKHomeFindSeat(PKHome* head);

PKHome* CreateSeat(PKHome** phead);

void PKHomeDestroy(PKHome** phead, PKHome* seat);
