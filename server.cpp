#include"common.h"

using namespace std;

void synchronize(); //TCP thread function

vector<unsigned> recieved_packets_ids; //global to share between TCP and UDP threads


int main(int argc, char* argv[])
{
    ios_base::sync_with_stdio(false); //make std::cout more thread friendly

    cout << "starting server." << endl;

    Socket udp_sock = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if(udp_sock<0){
        cerr << "can't open the UDP socket." << endl;
        return -1;
    }

    recieved_packets_ids.reserve(Packet::NUMBER_OF_PACKETS);


    Address server_address(Address::Type::local_server, udp_sock);
    Address client_address(Address::Type::remote);
    unsigned char connection_buffer[Packet::LENGTH];
        memset(&connection_buffer,0,Packet::LENGTH); //nullify the array (initialization)


    //notify the client with recieved packets status when he asks.
    thread tcp_thread(synchronize);

    //UDP listener in main thread
    while(1)
    {
        //get UDP packet
        if(recvfrom(udp_sock, connection_buffer,Packet::LENGTH,0,(sockaddr*)client_address.getReference(),client_address.getLengthReference()) < 0){
            cerr << "can't recieve UDP packet " << endl;
            return -1;
        }
        //save the recieved UDP packet id
        recieved_packets_ids.push_back(Packet::getID(connection_buffer));
        cout << "recieved packet with id: " << recieved_packets_ids.back() << endl;
    }

    return 0;
}

void synchronize()
{
    int* connection_buffer = new int;
    Socket tcp_sock = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
    Address server_address(Address::Type::local_server, tcp_sock);
    Address client_address(Address::Type::remote);
    Socket new_sock = server_address.waitForConnectionTCP(client_address); //establish TCP connection

    //read sync request from client and respond with the recieved IDs
    while(1)
    {
        if(read(new_sock,connection_buffer,sizeof(int)) < 0){
            cerr << "can't read transferred data from connection." << endl;
            exit(-1);
        }

        switch(*connection_buffer)
        {
            case (int)Signal::sync:
            {
                cout << "[TCP]Synchronization request from client. sending recieved IDs." << endl;
                for(auto itr = recieved_packets_ids.begin(); itr!=recieved_packets_ids.end();){
                    cout << '\t' << "sending ID: " << *itr << endl;
                    if(write(new_sock,&(*itr),sizeof(int)) < 0){ //itr isnt a pointer to int, so &(*itr) casts it to one.
                        cerr << "can't reply ID sync to client." << endl;
                        exit(-1);
                    }
                    itr = recieved_packets_ids.erase(itr); //remove the ID's from the list, as client is aknowledged now on.
                }

                //terminate the sync by sending null ID.
                Signal terminator = Signal::terminator;
                if(write(new_sock,&terminator,sizeof(int)) < 0){ //itr isnt a pointer to int, so &(*itr) casts it to one.
                    cerr << "can't send \"end of sync\" signal to client." << endl;
                    exit(-1);
                }
                cout << "[TCP]End of sync." << endl;
                break;
            }

            case (int)Signal::server_stop:
            {
                cout << "Recieved server shutdown command. exiting." << endl;
                exit(0);
            }
        }
    }
}
