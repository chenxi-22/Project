.PHONY:all
all:client server

client:client.c common.c  Game.c
	gcc -o $@ $^ -lpthread
server:server.c common.c  Game.c PKHome.c Person.c GameInformation.c ThreadPool.c
	gcc -g -o  $@ $^ -lpthread

.PHONY:clean

clean:
	rm -f server client
