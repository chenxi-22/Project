#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>

int64_t kk_start_up(int port); 

int sigpipe_handler();

int kk_read_config(char* config_path, char* port, char* root);

int accepet_handler(int64_t fd);

int set_no_block(int fd);
