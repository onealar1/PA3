#include "packet.h"
#include "circularQueue.h"

// Code from : https://www.geeksforgeeks.org/introduction-to-circular-queue/

using namespace  std;

void CircularQueue::enQueue(Packet* pkt){
    if((front == 0 & rear == size - 1) || ((rear+1) % size == front)){
        printf("Queue is full\n");
        return;
    }

    else if (front == -1){
        front = rear = 0;
        pkt_arr[rear] = pkt;
    }

    else if (rear == size - 1 && front != 0){
        rear = 0;
        pkt_arr[rear] = pkt;
    }

    else{
        rear++;
        pkt_arr[rear] = pkt;
    }
}

Packet* CircularQueue::deQueue(){
    if (front == -1){
        printf("Queue is empty\n");
        return NULL;
    }
    Packet* pkt = pkt_arr[front];
    pkt_arr[front] = NULL;

    if(front == rear){
        front = rear = -1;
    }

    else if(front == size - 1){
        front = 0;
    }

    else{
        front++;
    }
    return pkt;
}

void CircularQueue::displayQueue(){
    if(front == -1){
        printf("Queue is empty\n");
        return;
    }

    printf("Elements in window are: \n");
    if (rear >= front){
        for(int i = front; i <= rear; i++){
            pkt_arr[i]->printPacket();
        }
    }
    else{
        for(int i = front; i < size; i++){
            pkt_arr[i]->printPacket();
        }
        for(int i = 0; i <= rear; i++){
            pkt_arr[i]->printPacket();
        }
    }
}

Packet* CircularQueue::getItem(int ix){
    return pkt_arr[ix];
}

int CircularQueue::getFront(){
    return front;
}

int CircularQueue::getRear(){
    return rear;
}

int CircularQueue::getSize(){
    if (front == -1){
        //printf("Queue is empty\n");
        return 0;
    }

    else if (rear >= front){
        return rear - front + 1;
    }
    else {
        return (size - front) + rear + 1;
    }
}
