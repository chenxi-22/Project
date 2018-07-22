#include "GameInformation.h"

/////////////////////////////////////////////////////
// GameInformation
/////////////////////////////////////////////////////

void GameInfInit(GameInf** phead)
{
  if (phead == NULL) {
    return;
  }
  *phead = NULL;
}

GameInf* GameInfCreate(Local* l, int64_t sock, Person* head_person)
{
  GameInf* tmp= (GameInf*)malloc(sizeof(GameInf));

  tmp->sock = sock;
  tmp->s = STAY;
  tmp->p = FindLocalPerson(head_person, l);
  tmp->next = NULL;

  return tmp;
}

void GameInfPush(GameInf** phead, Local* l, int64_t sock, Person* head_person)
{
  if (phead == NULL || l == NULL) {
    return;
  }
  if (*phead == NULL) {
    *phead = GameInfCreate(l, sock, head_person);
    return;
  }
  GameInf* cur = *phead;
  while (cur->next != NULL) {
    cur = cur->next;
  }
  cur->next = GameInfCreate(l, sock, head_person);
}

GameInf* GameInfFind(GameInf* head, int64_t sock)
{
  if (head == NULL) {
    return NULL;
  }
  GameInf* cur = head;
  while (cur != NULL) {
    if (cur->sock == sock) {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

void GameInfDestroy(GameInf** phead, GameInf* pos)
{
  if (phead == NULL || pos == NULL) {
    return;
  }
  GameInf* cur = *phead;
  GameInf* pre = *phead;
  while (cur != NULL) {
    if (cur == pos) {
      if (cur == pre) {
        *phead = cur->next;
      } else {
        pre->next = cur->next;
      }
      free(cur->p);
      free(cur);
      return;
    }
    pre = cur;
    cur = cur->next;
  }
  return;
}

GameInf* AddReadyQueue(GameInf** phead_ready, GameInf** phead_stay, int64_t sock)
{
  if (phead_stay == NULL || phead_ready == NULL) {
    return NULL;
  }

  GameInf* stay = *phead_stay;
  GameInf* ready = *phead_ready;

  while (ready != NULL && ready->next != NULL) {
    ready = ready->next;
  }

  GameInf* pre = *phead_stay;
  while (stay != NULL) {
    if (stay->sock == sock) {
      if (stay == pre) {
        *phead_stay = stay->next;
      } else {
        pre->next = stay->next;
      }
      stay->next = NULL;
      if (ready == NULL) {
        *phead_ready = stay;
      } else {
        ready->next = stay;
      }
      return stay;
    }
    pre = stay;
    stay = stay->next;
  }
  return NULL;
}

GameInf* LoginFind(GameInf* head, Local* l)
{
  if (head == NULL) {
    return NULL;
  }
  GameInf* cur = head;
  while (cur != NULL) {
    if (strcmp(cur->p->id, l->id_buf) == 0) {
      if (strcmp(cur->p->passwd, l->passwd_buf) == 0) {
        return cur;
      }
    }
    cur = cur->next;
  }
  return NULL;
}

GameInf* AddStayQueue(GameInf** phead_stay, GameInf** phead_ready, int64_t sock)
{
  if (phead_stay == NULL || phead_ready == NULL) {
    return NULL;
  }

  GameInf* stay = *phead_stay;
  GameInf* ready = *phead_ready;

  while (stay != NULL && stay->next != NULL) {
    stay = stay->next;
  }

  GameInf* pre = *phead_ready;
  while (ready != NULL) {
    if (ready->sock == sock) {
      if (ready == pre) {
        *phead_ready = ready->next;
      } else {
        pre->next = ready->next;
      }
      ready->next = NULL;
      if (stay == NULL) {
        *phead_stay = ready;
      } else {
        stay->next = ready;
      }
      return ready;
    }
    pre = ready;
    ready = ready->next;
  }
  return NULL;
}
