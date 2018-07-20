#include <signal.h>
#include "common.h"
#include "Game.h"
#include "GameInformation.h"
#include "PKHome.h"
#include "Person.h"

Person* head_person; // 游戏内信息链表头
GameInf* head_stay; // stay 队列
GameInf* head_ready; // ready 队列
PKHome* head_pkhome; // home 

pthread_mutex_t person_mutex; // 个人信息互斥量
pthread_mutex_t ready_mutex; // 就绪队列互斥量
pthread_mutex_t stay_mutex; // 游戏大厅互斥量
pthread_mutex_t pkhome_mutex; // 游戏房间互斥量

static void* HandlerRequest(void* arg)
{
  int64_t sock = (int64_t)arg;

  Local l;

  while (1) { // 处理登录界面的请求
    read(sock, &l, sizeof(l));
    if (l.ls == LOGIN) { // 登录
      Person* ret = FindLocalPerson(head_person, &l);
      if (ret != NULL) {
        write(sock, "yes", 3);
        pthread_mutex_lock(&stay_mutex);
        GameInfPush(&head_stay, &l, sock, head_person);
        pthread_mutex_unlock(&stay_mutex);
        break;
      } else {
        write(sock, "no", 2);
      }
    } else if(l.ls == REGISTE) { // 注册
      bool ret = FindLocalPerson(head_person, &l);
      if (ret) {
        write(sock, "no", 2);
      } else {
        write(sock, "yes", 3);

        pthread_mutex_lock(&person_mutex);
        PersonPush(&head_person, &l);
        pthread_mutex_unlock(&person_mutex);

        pthread_mutex_lock(&stay_mutex);
        GameInfPush(&head_stay, &l, sock, head_person);
        pthread_mutex_unlock(&stay_mutex);
        break;
      }
    } else { // 退出
      goto end;
    }
  }

  Coord c; // 定义坐标
  while (1) {
    read(sock, &l, sizeof(l));
    if (l.ls == EXIT) { // 退出
      goto end;
    }
    pthread_mutex_lock(&ready_mutex);
    pthread_mutex_lock(&stay_mutex);
    GameInf* person_node = AddReadyQueue(&head_ready, &head_stay, sock);
    pthread_mutex_unlock(&stay_mutex);
    pthread_mutex_unlock(&ready_mutex);

    person_node->s = READY;
    pthread_mutex_lock(&pkhome_mutex);
    PKHome* seat = PKHomeFindSeat(head_pkhome);
    if (seat == NULL) {
      seat = CreateSeat(&head_pkhome);
      seat->p1 = person_node;
    } else {
      if (seat->p1 == NULL) {
        seat->p1 = person_node;
      } else if (seat->p2 == NULL){
        seat->p2 = person_node;
      }
    }
    pthread_mutex_unlock(&pkhome_mutex);

    while (1) { // Mark:beg
      while (seat->p1 == NULL || seat->p2 == NULL) {
      }

      if (seat->p1 != NULL && seat->p1->sock == sock) {
        seat->p1->s = PLAY;
      }
      if (seat->p2 != NULL && seat->p2->sock == sock) {
        seat->p2->s = PLAY;
      }

      while (seat->p1 == NULL || seat->p2 == NULL ||
             seat->p1->s!= PLAY || seat->p2->s != PLAY) {
      }

      if (seat->p1->sock == sock) {
        write(seat->p1->sock, "first", 5);
      } 
      if (seat->p2->sock == sock) {
        write(seat->p2->sock, "second", 6);
      }

      c.win_sock = 0;

      while (1) {
        if (seat->p1->sock == sock){
          read(seat->p2->sock, &c, sizeof(c));
          write(seat->p1->sock, &c, sizeof(c));
          if (c.win_sock != 0) {
            seat->p1->s = FINISH;
            seat->p2->s = FINISH;
            break;
          }
        } 
        if (seat->p2->sock == sock) {
          read(seat->p1->sock, &c, sizeof(c));
          write(seat->p2->sock, &c, sizeof(c));
          if (c.win_sock != 0) {
            seat->p1->s = FINISH;
            seat->p2->s = FINISH;
            break;
          }
        } 
      } // end while (1)

      read(sock, &l, sizeof(l));

      if (l.ls == EXIT) { // 退出游戏
        if (seat->p1->sock == sock) {
          write(seat->p1->sock, "ok", 2);
        } 
        if (seat->p2->sock == sock) {
          write(seat->p2->sock, "ok", 2);
        }
        goto end;
      } else if (l.ls == RETURN) { // 返回游戏大厅
        if (seat->p1 != NULL && seat->p1->sock == sock) {
          seat->p1->s = STAY;
          seat->p1 = NULL;
        } else {
          seat->p2->s = STAY;
          seat->p2 = NULL;
        }
        write(sock, "ok", 2);

        pthread_mutex_lock(&ready_mutex);
        pthread_mutex_lock(&stay_mutex);
        AddStayQueue(&head_stay, &head_ready, sock);
        pthread_mutex_unlock(&ready_mutex);
        pthread_mutex_unlock(&stay_mutex);

        break;
      } else { // 继续游戏
        if (seat->p1->sock == sock) {
          seat->p1->s = READY;
          write(seat->p1->sock, "ok", 2);
        } 
        if (seat->p2->sock == sock) {
          seat->p2->s = READY;
          write(seat->p2->sock, "ok", 2);
        }
      }
    } // end while (1) Mark:beg
    // if (seat->p1 == NULL && seat->p2 == NULL) {
    //   PKHomeDestroy(&head_pkhome, seat);
    // }
  }

end:
  printf("Client Quit!\n");
  GameInf* ret = GameInfFind(head_stay, sock);
  pthread_mutex_lock(&stay_mutex);
  GameInfDestroy(&head_stay, ret);
  pthread_mutex_unlock(&stay_mutex);
  close(sock);

  return NULL;
}

void Handler(int a)
{
  a = 0;
}

int main(int argc, char* argv[])
{
  if (Usage(argc, argv) == 0) {
    return 1;
  }

  signal(SIGPIPE, Handler);

  int listen_sock = ServerStartUp(atoi(argv[1]));

  PersonInit(&head_person);
  GameInfInit(&head_stay);
  GameInfInit(&head_ready);
  PKHomeInit(&head_pkhome);

  pthread_mutex_init(&stay_mutex, NULL);
  pthread_mutex_init(&person_mutex, NULL);
  pthread_mutex_init(&ready_mutex, NULL);
  pthread_mutex_init(&pkhome_mutex, NULL);

  while (1) {
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int64_t sock = GetConnect(listen_sock, (struct sockaddr*)&client, len);

    pthread_t tid;
    pthread_create(&tid, NULL, HandlerRequest, (void*)sock);
    pthread_detach(tid);
  }

  return 0;
}
