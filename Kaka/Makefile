CC = gcc

server:main.c comm.c echo_error_log.c epoll_operations.c http_response.c thread_pool.c util.c cgi_response.c normal_response.c timer.c
	$(CC) -g -o $@ $^ -lpthread

.PHONY:clean
clean:
	rm -f *.o server
