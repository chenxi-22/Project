#include "Game.h"

/////////////////////////////////////////////////////
// Person
/////////////////////////////////////////////////////

void PersonInit(Person** phead) // 初始化
{
  if (phead == NULL) {
    return;
  }
  *phead = NULL;
}

Person* PersonCreate(Local* l) // 创建
{
  Person* tmp = (Person*)malloc(sizeof(Person));

  strcpy(tmp->id, l->id_buf);
  strcpy(tmp->passwd, l->passwd_buf);
  tmp->next = NULL;

  return tmp;
}

void PersonPush(Person** phead,Local* l) // 添加
{
  if (phead == NULL) {
    return;
  }
  if (*phead == NULL) {
    *phead = PersonCreate(l);
    return;
  }
  Person* cur = *phead;
  while (cur->next != NULL) {
    cur = cur->next;
  }
  cur->next = PersonCreate(l);
  return;
}

Person* FindLocalPerson(Person* head, Local* l) // 查询
{
  Person* cur = head;
  while (cur != NULL) {
    if ((strcmp(cur->id, l->id_buf) == 0) &&
        (strcmp(cur->passwd, l->passwd_buf) == 0)) {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}


