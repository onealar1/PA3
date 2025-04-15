#ifndef HOST_H
#define HOST_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <iostream>

#define MAX_PAYLOAD_SIZE 50
#define MAX_WIN_SIZE 6
#define MAX_SEQ_SIZE 10

struct sock_listener {
    int sockfd;
    struct addrinfo* p;
};

struct sock_talker {
    int sockfd;
    struct addrinfo* p;
};

#endif