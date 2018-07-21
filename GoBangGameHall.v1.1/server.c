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

#define ALIVETIME 30 // 心跳包每 30 s 发送一次
#define ALIVEINTVL 5 // 如果没有接收到返回，那么接下来每 5 s 发送一次
#define ALIVEPROBES 3 // 总共发 3 次

static void* KeepAliveRequest(void* arg) // 心跳包
{
  int64_t sock = (int64_t)arg;
  char send_buf[32] = "Are you alive?";

  while (1) {
    sleep(ALIVETIME);
    write(sock, send_buf, strlen(send_buf));
    ssize_t s = read(sock, send_buf, sizeof(send_buf) - 1);
    if (s == 0) {
      int count = 1;
      while (1) { // Mark beg
        sleep(ALIVEINTVL);
        write(sock, send_buf, strlen(send_buf));
        s = read(sock, send_buf, sizeof(send_buf) - 1);
        count++;
        if (s > 0) {
          break;
        }
        if (count >= ALIVEPROBES) {
          close(sock);
        }
      } // Mark end while (1)
    }
  }

  return NULL;
}

static void* HandlerRequest(void* arg) // 主执行
{
  int64_t sock = (int64_t)arg;
  ssize_t s = 0;

  GameInf* ret_stay = NULL; 
  GameInf* ret_ready = NULL; 
  Local l;

  while (1) { // 处理登录界面的请求
    s = read(sock, &l, sizeof(l));
    if (s <= 0) {
      EchoErrorLog();
      perror("read");
      goto end;
    }

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
    s = read(sock, &l, sizeof(l));
    if (s <= 0) {
      EchoErrorLog();
      perror("read");
      goto end;
    }

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
          s = read(seat->p2->sock, &c, sizeof(c));
          if (s <= 0) {
            EchoErrorLog();
            perror("read");
            goto end;
          }

          write(seat->p1->sock, &c, sizeof(c));
          if (c.win_sock != 0) {
            seat->p1->s = FINISH;
            seat->p2->s = FINISH;
            break;
          }
        } 
        if (seat->p2->sock == sock) {
          s = read(seat->p1->sock, &c, sizeof(c));
          if (s <= 0) {
            EchoErrorLog();
            perror("read");
            goto end;
          }

          write(seat->p2->sock, &c, sizeof(c));
          if (c.win_sock != 0) {
            seat->p1->s = FINISH;
            seat->p2->s = FINISH;
            break;
          }
        } 
      } // end while (1)

      s = read(sock, &l, sizeof(l));
      if (s <= 0) {
        EchoErrorLog();
        perror("read");
        goto end;
      }

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
  ret_stay = GameInfFind(head_stay, sock);
  ret_ready = GameInfFind(head_ready, sock);
  pthread_mutex_lock(&stay_mutex);
  GameInfDestroy(&head_stay, ret_stay);
  GameInfDestroy(&head_ready, ret_ready);
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

  int fd = open("Error.log", O_RDWR | O_CREAT | O_APPEND, 0666);
  dup2(fd, 2);

  int handler_listen_sock = ServerStartUp(atoi(argv[1]));
  int kpalive_listen_sock = ServerStartUp(atoi(argv[2]));

  PersonInit(&head_person);
  GameInfInit(&head_stay);
  GameInfInit(&head_ready);
  PKHomeInit(&head_pkhome);

  pthread_mutex_init(&stay_mutex, NULL);
  pthread_mutex_init(&person_mutex, NULL);
  pthread_mutex_init(&ready_mutex, NULL);
  pthread_mutex_init(&pkhome_mutex, NULL);

  while (1) {
    struct sockaddr_in client_1; // 为了客户端主执行任务
    socklen_t len1 = sizeof(client_1);

    struct sockaddr_in client_2; // 为了心跳包的执行任务
    socklen_t len2 = sizeof(client_2);

    int64_t handler_sock = GetConnect(handler_listen_sock, (struct sockaddr*)&client_1, len1);
    int64_t kpalive_sock = GetConnect(kpalive_listen_sock, (struct sockaddr*)&client_2, len2);

    pthread_t tid1;
    pthread_t tid2;
    pthread_create(&tid1, NULL, HandlerRequest, (void*)handler_sock); // 主执行任务
    pthread_create(&tid2, NULL, KeepAliveRequest, (void*)kpalive_sock); // 主执行任务
    pthread_detach(tid1);
    pthread_detach(tid2);
  }

  return 0;
}
