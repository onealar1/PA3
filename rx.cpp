#include "packet.cpp"
#include "circularQueue.cpp"
#include "host.h"
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>

using namespace std;

sock_listener listener;
sock_talker talker;

void init_listener(char* tx_port){

    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, tx_port, &hints, &servinfo)) != 0){
        fprintf(stderr, "gaiaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
    for(listener.p = servinfo; listener.p != NULL; listener.p = listener.p->ai_next){

        if((listener.sockfd = socket(listener.p->ai_family, listener.p->ai_socktype, listener.p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }
         
        if(bind(listener.sockfd, listener.p->ai_addr, listener.p->ai_addrlen) == -1){
            close(listener.sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if(listener.p == NULL){
        fprintf(stderr, "listener failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);

}

void init_talker(char* rx_addr, char* rx_port){

    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if((rv = getaddrinfo(rx_addr, rx_port, &hints, &servinfo)) != 0){
        fprintf(stderr, "gaiaddrinfo %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    for(talker.p = servinfo; talker.p != NULL; talker.p = talker.p->ai_next){
        if((talker.sockfd = socket(talker.p->ai_family, talker.p->ai_socktype, talker.p->ai_protocol)) == -1){
            perror("talker: socket\n");
            continue;
        }
        break;
    }

    if (talker.p == NULL){
        fprintf(stderr, "talker: failed to create socket\n");
        exit(EXIT_FAILURE);
    }
}

void reliablyReceive(char* rx_addr, char* rx_port, char* tx_port, char* filename, int bytesToTransfer){

    init_listener(tx_port);
    init_talker(rx_addr, rx_port);

    /*
    insert your code here
    Feel free to create other functions for better code design. 
    */

    close(listener.sockfd);
    close(talker.sockfd);
}

int main(int argc, char* argv[]){

    if(argc != 5){
        fprintf(stderr, "invalid number of arguments");
        exit(EXIT_FAILURE);
    }
    
    char* filepath = argv[1];
    char* tx_port = argv[2];
    char* rx_addr = argv[3];
    char* rx_port = argv[4];

    reliablyReceive(rx_addr, rx_port, tx_port, filepath, 50);

    return 0;
}