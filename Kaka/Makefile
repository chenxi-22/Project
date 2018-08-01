CC = gcc

server:main.c Comm.c EchoErrorLog.c EpollOperation.c HttpResponse.c ThreadPool.c Util.c CgiResponse.c NormalResponse.c
	$(CC) -o $@ $^ -lpthread

.PHONY:clean
clean:
	rm -f *.o server
