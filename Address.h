#ifndef ADDRESS_H
#define ADDRESS_H

#include<iostream>
#include<unistd.h>
#include<cstring>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>


typedef int Socket; //socketfd

using namespace std;

class Address
{
    static constexpr int port = 1337;

    sockaddr_in raw_c_address;
    socklen_t length;
    Socket sock;


public:
    enum class Type {local_server, remote};

    Address(Type type, Socket socket);
        Address(Type type) :Address(type,0){} //with default socket argument

    void nullify();
    Socket waitForConnectionTCP(Address &client_address);
    void readFromClient();

    void setIP(hostent* address_info);

    sockaddr_in* getReference();
    Socket getSocket();
    socklen_t* getLengthReference();
};



#endif

