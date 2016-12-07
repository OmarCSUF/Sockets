#include"Address.h"




Address::Address(Type type, Socket socket)
{
    sock = socket;
    nullify(); //initialize server address with zeroes;
    raw_c_address.sin_family = AF_INET; //set as web socket (not OS socket)
    raw_c_address.sin_port = htons(port); //set port
    raw_c_address.sin_addr.s_addr = (INADDR_ANY);

    if(type == Type::local_server){
        if(bind(sock,(sockaddr*)&raw_c_address, sizeof(raw_c_address)) < 0){
            cerr << "bad bind." << endl;
            exit(-1);
        }
    }

    length = sizeof(sockaddr_in);
}


void Address::nullify()
{
    memset(&raw_c_address,0,sizeof(raw_c_address));
}

Socket Address::waitForConnectionTCP(Address &client_address)
{
    listen(sock, 5); //listen for inbound connections
    socklen_t client_address_size = sizeof(*client_address.getReference());
    //block thread until conection established, save client data.
    sock = accept(sock, (sockaddr*)client_address.getReference(), &client_address_size);
    if(sock < 0){
        cerr << "can't accept connection/s" << endl;
        exit(-1);
    }
    return sock;
}

void Address::setIP(hostent* address_info)
{
    raw_c_address.sin_addr = *((in_addr *)address_info->h_addr); //set the ip
    memset(&(raw_c_address.sin_zero), '\0', 8);  //zero the rest of the struct
}


sockaddr_in* Address::getReference()
{
    return &raw_c_address;
}

Socket Address::getSocket()
{
    return sock;
}

socklen_t* Address::getLengthReference()
{
    return &length;
}
