#include"Packet.h"

int thread_local Packet::success_count = 0;
int thread_local Packet::loss_count = 0;
int thread_local Packet::deliver_count = 0;
int thread_local Packet::over_limit_count = 0;

void Packet::progressLossCount()
{
    loss_count++;
}

void Packet::progressDeliverCount()
{
    deliver_count++;
}

unsigned Packet::getID(unsigned char* raw_packet)
{
    return raw_packet[0] + (((int)raw_packet[1])<<8) + (((int)raw_packet[2])<<16) + (((int)raw_packet[3])<<24);
}

Packet::Packet()
{
    raw = new unsigned char[LENGTH];
    retry_count = 0;
    state = State::neutral; //nor success nor failure just yet
}

Packet::Packet(unsigned id) : Packet()
{
    generatePacket(id);
}

Packet::~Packet()
{
    if(state == State::success){
        success_count++;
    }else if(state == State::over_limit){
        over_limit_count++;
    }
}


void Packet::generatePacket(unsigned id)
{
    memset(raw,'\0',LENGTH); //init packet with null data

    //save id inside the packet bytes
    raw[0] = (id)     & (unsigned)0xff;
    raw[1] = (id>>8)  & (unsigned)0xff;
    raw[2] = (id>>16) & (unsigned)0xff;
    raw[3] = (id>>24) & (unsigned)0xff;


    this->id=id; //save id for class instance inner logics
}

void Packet::progressRetryCount()
{
    retry_count++;
}

void Packet::setState(Packet::State new_state)
{
    state = new_state;
}

unsigned char* Packet::getReference()
{
    return raw;
}

int Packet::getRetryCount()
{
    return retry_count;
}

unsigned Packet::getID()
{
    return getID(raw);
}

Packet::State Packet::getState()
{
    return state;
}
