#ifndef PACKET_H
#define PACKET_H

// This is a helper code; however, it's not stable file. So feel free to change this file if you need some other functionality

class Packet {
    private:
        int flag; // 1 for data packet, 0 for ACK and 2 for FIN
        int seqnum; // seq number for data, ACK or FIN packets
        int length; // length for the payload; it will be 0 for ACK and FIN packets
        char* payload; // message inside the packet

    public:
        Packet(int flag, int seqnum, int length, char* payload);
        
        // getter methods
        int getFlag();
        int getSeqnum();
        int getLength();
        char* getPayload();

        /*
        @brief prints the content of a packet structure
        */
        void printPacket();

        /*
        @brief converts string packet into a Packet structure
        @param a string: the contents of a packet. For eg: 1-0-0-hello
        */
        void serialize(char* strpkt);

        /*
        @brief converts string packet into a Packet structure
        @param a string: the contents of a packet. For eg: 1-0-0-hello
        */
        void deserialize(char* strpkt);
};

#endif