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

    // get addr info for transmitter port
    if((rv = getaddrinfo(NULL, tx_port, &hints, &servinfo)) != 0){
        fprintf(stderr, "gaiaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    // create socket and bind to the port and addr
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

    // get addr info for receiver port
    if((rv = getaddrinfo(rx_addr, rx_port, &hints, &servinfo)) != 0){
        fprintf(stderr, "gaiaddrinfo %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    // create socket
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

/**
 * Helper method to send an acknowledgment
 */
void sendACK(int ackSeq){
    Packet ackPkt(0, ackSeq, "");
    string serialized = ackPkt.serialize();
    sendto(talker.sockfd, serialized.c_str(), serialized.length(), 0, talker.p->ai_addr, talker.p->ai_addrlen);
}

void reliablyReceive(char* rx_addr, char* rx_port, char* tx_port, char* filename, int bytesToTransfer){

    init_listener(tx_port);
    init_talker(rx_addr, rx_port);

    char buffer[1024];
    int expectedSeq = 0;
    ofstream outfile(filename, ios::binary);

    // loop until all is received
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        socklen_t addr_len = sizeof(listener.p->ai_addr);
        int bytesRec = recvfrom(listener.sockfd, buffer, sizeof(buffer), 0, listener.p->ai_addr, &addr_len);

        string raw(buffer, bytesRec);
        Packet pkt;
        pkt.deserialize(buffer);

        int flag = pkt.getFlag();
        int seq = pkt.getSeqnum();

        if (flag == 1 && seq == expectedSeq) { // data packet and expected
            outfile.write(pkt.getPayload().c_str(), pkt.getLength());
            sendACK(seq);
            expectedSeq = (expectedSeq + 1) % 10;
        } 
        else if (flag == 1) { // data packet but not expected
            sendACK((expectedSeq - 1 + 10) % 10); 
        } 
        else if (flag == 2) { // FIN packet
            sendACK(seq); 
            break;
        }
    }

    outfile.close();
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