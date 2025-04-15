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

    // get addr info for transmiter port
    if((rv = getaddrinfo(NULL, tx_port, &hints, &servinfo)) != 0){
        fprintf(stderr, "gaiaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    // create socket, make the socket non blocking and bind the socket to the addr and port
    for(listener.p = servinfo; listener.p != NULL; listener.p = listener.p->ai_next){

        // create socket
        if((listener.sockfd = socket(listener.p->ai_family, listener.p->ai_socktype, listener.p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }
         
        // get the flags
        int flags = fcntl(listener.sockfd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl F_GETFL");
            close(listener.sockfd);
            continue;
        }

        // make socket non blocking
        if (fcntl(listener.sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl F_SETFL O_NONBLOCK");
            close(listener.sockfd);
            continue;
        }

        // bind socket to addr and port
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
    hints.ai_socktype = SOCK_DGRAM; // udp

    // get addr info for receiver port
    if((rv = getaddrinfo(rx_addr, rx_port, &hints, &servinfo)) != 0){
        fprintf(stderr, "gaiaddrinfo %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    // create socket on the first port available
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

// sets a timeout on the socket
void timeout(int sockfd, int timeoutTime){
    struct timeval timeout;
    timeout.tv_sec = timeoutTime;
    timeout.tv_usec = 0;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
}

/**
 *  Helper method to slide the window
 */
int slideWindow(CircularQueue& window, int ackSeq, int& firstSeqNum, int maxSeqNums) {
    int ackedBytes = 0;

    while (window.getSize() > 0) {
        Packet* frontPkt = window.getItem(window.getFront());
        int frontSeq = frontPkt->getSeqnum();

        if ((frontSeq <= ackSeq && firstSeqNum <= ackSeq) ||
            (frontSeq > ackSeq && firstSeqNum > ackSeq)) {

            ackedBytes += frontPkt->getLength();
            delete window.deQueue();
            firstSeqNum = (firstSeqNum + 1) % maxSeqNums;
        } else {
            break;
        }
    }

    return ackedBytes;
}

void reliablyTransfer(char* rx_addr, char* rx_port, char* tx_port, char* filename, int bytesToTransfer){

    init_listener(tx_port);
    init_talker(rx_addr, rx_port);

    /*
    insert your code here
    Feel free to create other functions for better code design. 
    */
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    const int winSize = 6;
    const int maxSeqNums = 10;
    const int payloadSize = 1000;

    CircularQueue window(winSize);

    int nextSeqNum = 0;
    int firstSeqNum = 0;
    int totalSent = 0;
    int totalAcked = 0;
    char buffer[payloadSize + 1];

    timeout(listener.sockfd, 5);

    // add packets to window
    char buffer[payloadSize + 1];

    while (window.getSize() < window.getCapacity() && totalSent < bytesToTransfer) {
        int toRead = std::min(payloadSize, bytesToTransfer - totalSent);
        int bytesRead = fread(buffer, sizeof(char), toRead, file);
        buffer[bytesRead] = '\0';

        Packet* pkt = new Packet(1, nextSeqNum, bytesRead, strdup(buffer));
        char pktStr[1100];
        pkt->serialize(pktStr);

        sendto(talker.sockfd, pktStr, strlen(pktStr), 0, talker.p->ai_addr, talker.p->ai_addrlen);
        window.enQueue(pkt);
        totalSent += bytesRead;
        nextSeqNum = (nextSeqNum + 1) % maxSeqNums;
    }

    // sending ACKs
    while (totalAcked < bytesToTransfer) {
        char ackBuffer[100];
        int ackLen = recvfrom(listener.sockfd, ackBuffer, sizeof(ackBuffer), 0, NULL, NULL);

        // handling timeout
        if (ackLen == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                cout << "Timeout. Retransmitting window...\n";

                int front = window.getFront();
                int count = window.getSize();

                for (int i = 0; i < count; i++) {
                    int index = (front + i) % winSize;
                    Packet* pkt = window.getItem(index);
                    char pktStr[1100];
                    pkt->serialize(pktStr);
                    sendto(talker.sockfd, pktStr, strlen(pktStr), 0, talker.p->ai_addr, talker.p->ai_addrlen);
                }
            }
        } 
        else {
            ackBuf[ackLen] = '\0';
            Packet ackPkt(0, 0, 0, NULL);
            ackPkt.deserialize(ackBuffer);

            int ackSeq = ackPkt.getSeqnum();

            int ackedBytes = slideWindow(window, ackSeq, firstSeqNum, maxSeqNums);
            totalAcked += ackedBytes;

            // add new packets to window
            while (window.getSize() < winSize && totalSent < bytesToTransfer) {
                int toRead = min(payloadSize, bytesToTransfer - totalSent);
                int bytesRead = fread(buffer, sizeof(char), toRead, file);
                buffer[bytesRead] = '\0';

                Packet* pkt = new Packet(1, nextSeqNum, bytesRead, strdup(buffer));
                char pktStr[1100];
                pkt->serialize(pktStr);

                sendto(talker.sockfd, pktStr, strlen(pktStr), 0, talker.p->ai_addr, talker.p->ai_addrlen);
                window.enQueue(pkt);
                totalSent += bytesRead;
                nextSeqNum = (nextSeqNum + 1) % maxSeqNums;
            }
        }

    fclose(file);
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

    reliablyTransfer(rx_addr, rx_port, tx_port, filepath, 50);

    return 0;

}