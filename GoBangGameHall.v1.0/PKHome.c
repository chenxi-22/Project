#include "Game.h"

/////////////////////////////////////////////////////
// PKHome
/////////////////////////////////////////////////////

void PKHomeInit(PKHome** phead)
{
  if (phead == NULL) {
    return;
  }
  *phead = NULL;
}

PKHome* PKHomeFindSeat(PKHome* head) 
{
  if (head == NULL) {
    return NULL;
  }
  PKHome* cur = head;
  while (cur != NULL) {
    if (cur->p1 == NULL || cur->p2 == NULL) {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

PKHome* CreateSeat(PKHome** phead)
{
  if (phead == NULL) {
    return NULL;
  }

  PKHome* cur = *phead;
  while (cur != NULL && cur->next != NULL) {
    cur = cur->next;
  }

  PKHome* tmp = (PKHome*)malloc(sizeof(PKHome));
  tmp->p1 = NULL;
  tmp->p2 = NULL;
  if (cur == NULL) {
    *phead = tmp;
  } else {
    cur->next = tmp;
  }
  return tmp;
}

void PKHomeDestroy(PKHome** phead, PKHome* seat)
{
  if (phead == NULL) {
    return;
  }
  if (*phead == NULL) {
    return;
  }

  PKHome* cur = *phead;
  PKHome* pre = *phead;
  while (cur != NULL) {
    if (cur == seat) {
      break;
    }
    pre = cur;
    cur = cur->next;
  }

  if (pre == cur) {
    free(cur);
    *phead = NULL;
  } else {
    pre->next = cur->next;
    free(cur);
  }
  return;
}


