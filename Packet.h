#ifndef PACKET_H
#define PACKET_H

#include<cstdio>
#include<cstring>

class Packet
{
public:
    enum class State {neutral, success, over_limit}; //shows if the packet was recieved, dumped, etc

private:
    int id;
    unsigned char* raw;
    int retry_count;
    State state;


public:

    static constexpr int NUMBER_OF_PACKETS = 10000;
    static constexpr int LENGTH = 1000;
    static constexpr double LOSS_PROBABILITY = 0.66;
    static constexpr int RETRY_LIMIT = 3;
    static thread_local int success_count;
    static thread_local int loss_count;
    static thread_local int deliver_count;
    static thread_local int over_limit_count;
    static void progressLossCount();
    static void progressDeliverCount();
    static unsigned getID(unsigned char* raw_packet);

    Packet();
    Packet(unsigned id);
    ~Packet();

    void generatePacket(unsigned id);
    void progressRetryCount();
    void setState(State new_state);

    unsigned char* getReference();
    int getRetryCount();
    unsigned getID();
    State getState();




};


#endif

