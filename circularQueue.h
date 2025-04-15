#ifndef CQUEUE_H
#define CQUEUE_H

#include "packet.h"
#include <stdlib.h>
#include <iostream>

// Code from : https://www.geeksforgeeks.org/introduction-to-circular-queue/

class CircularQueue {

    int rear, front;
    int size;
    Packet** pkt_arr;

    public:
        CircularQueue(int s){ 
            front = rear = -1; 
            size = s;
            pkt_arr = new Packet*[s];
        }

        void enQueue(Packet* pkt);
        Packet* deQueue();
        void displayQueue();
        int getSize();
        int getFront();
        int getRear();
        Packet* getItem(int ix);
};

#endif