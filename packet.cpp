#include "packet.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

Packet::Packet(int flag, int seqnum, int length, char* payload){
    this->flag = flag;
    this->seqnum = seqnum;
    this->length = length;
    this->payload = payload;
}

int Packet::getFlag(){
    return this->flag;
}

int Packet::getSeqnum(){
    return this->seqnum;
}

int Packet::getLength(){
    return this->length;
}

char* Packet::getPayload(){
    return this->payload;
}

void Packet::printPacket(){
    if(length > 0)
        cout << "Packet contains: <flag: " << flag << "; seqnum: " << seqnum << "; length: " << length << "; payload: " << payload << ">" << endl;
    else if (length == 0)
        cout << "Packet contains: <flag: " << flag << "; seqnum: " << seqnum << "; length: " << 0 << "; payload: NULL" << payload << ">" << endl;
    else{
        cout << "Packet length is negative. Invalid." << endl;
        exit(EXIT_FAILURE);
    }
}

void Packet::serialize(char* strpkt){
    sprintf(strpkt, "%d-%d-%d-%s", flag, seqnum, length, payload);
}

void Packet::deserialize(char* strpkt){
    char *token, *ptr;

    token = strtok(strpkt, "-"); // extract the flag 
    flag = strtol(token, &ptr, 10); // converts the flag from string to int type

    token = strtok(NULL, "-");
    seqnum = strtol(token, &ptr, 10);

    token = strtok(NULL, "-");
    length = strtol(token, &ptr, 10);

    if(length == 0){
        this->payload = NULL;
    }

    else if (length > 0){ 
        token = strtok(NULL, "");
        this->payload = new char[length];
        for(int i=0; i < length; i++){
            this->payload[i] = token[i];
        }
        this->payload[length] = '\0';
    }

    else {
        cout << "Payload length is negative. Invalid" << endl;
        exit(EXIT_FAILURE);
    }
}



