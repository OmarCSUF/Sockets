#include"common.h"

//for different srand()seeds for each thread
int getRandomSeed(){
    static int counter = time(0);
    return counter++;
}

void client(char* server_name)
{
    string SERVER_TAG = '['+string(server_name)+"]\t";

    Socket udp_sock = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
    if(udp_sock<0){
        cerr << SERVER_TAG << "can't open the UDP socket." << endl;
        exit(-1);
    }
    Socket tcp_sock = socket(AF_INET, SOCK_STREAM, 0); //TCP socket
    if(udp_sock<0){
        cerr << SERVER_TAG << "can't open the UDP socket." << endl;
        exit(-1);
    }

    Address server_udp_address(Address::Type::remote, udp_sock);
    Address server_tcp_address(Address::Type::remote, tcp_sock);
    hostent* server_info = gethostbyname(server_name);
    if(!server_info){
        cerr << SERVER_TAG << "no such address:" << server_name << endl;
        exit(-1);
    }
    server_udp_address.setIP(server_info); //give server_address the IP address
    server_tcp_address.setIP(server_info); //give server_address the IP address

    //establish TCP connection
    if(connect(server_tcp_address.getSocket(),(sockaddr*)server_tcp_address.getReference(),*server_tcp_address.getLengthReference()) < 0){
        cerr << SERVER_TAG << "can't establish TCP connection." << endl;
        exit(-1);
    }

    vector<Packet*> pending_packets;
    pending_packets.reserve(Packet::NUMBER_OF_PACKETS);

    //generate the packets
    for(unsigned i=1; i<=Packet::NUMBER_OF_PACKETS; i++){ //"i" will serve for packet ID generation (1-BYTES_IN_PACKET)
        pending_packets.push_back(new Packet(i));
    }

    srand(getRandomSeed()); //init random generator (for loss probability)

    //send packets
    while(!pending_packets.empty())
    {
        //for each remaining packet (yet not recieved by the server, not exceeded retry limit)
        for(auto packet_iterator = pending_packets.begin(); packet_iterator!=pending_packets.end(); packet_iterator++)
        {
            //check for retry count
            if((*packet_iterator)->getRetryCount() == Packet::RETRY_LIMIT){
                cout << SERVER_TAG << "packet id: "<< (*packet_iterator)->getID() << "\t has exceeded the retry limit: " << Packet::RETRY_LIMIT << endl;
                (*packet_iterator)->setState(Packet::State::over_limit);
                delete *packet_iterator;
                packet_iterator = pending_packets.erase(packet_iterator); //remove this packet from pending list.
                if(packet_iterator == pending_packets.end()){
                    break;
                }else{
                    continue;
                }
            }

            //send packet by UDP
            if(double(rand())/RAND_MAX > Packet::LOSS_PROBABILITY){  //if passed the "probability test" to send the packet
                if(sendto(server_udp_address.getSocket(),(*packet_iterator)->getReference(),Packet::LENGTH,0,(sockaddr*)server_udp_address.getReference(),*server_udp_address.getLengthReference()) < 0){
                    cerr << SERVER_TAG << "can't send UDP packet" << endl;;
                    exit(-1);
                }
            }else{ //simulation of loss:
                Packet::progressLossCount();
            }

            //mark as sent (for retry limit)
            (*packet_iterator)->progressRetryCount();
        }

        //get list of recieved packets in this "round". (TCP)/////////////////////////////////////////////
        //ask the server to sync in.
        Signal signal = Signal::sync;
        if(write(server_tcp_address.getSocket(),&signal,sizeof(int)) < 0){
            cerr << SERVER_TAG << "can't send TCP to server." << endl;
            exit(-1);
        }
        //here server starts to sync in the ID's of recieved UDP packets, one by one
        unsigned* sync_id_packet = new unsigned;
        int temp = 0;
        do
        {
            temp = read(server_tcp_address.getSocket(),sync_id_packet,sizeof(int)); //read packet ID's of the recieved by server
            if(temp < 0){
                cerr << SERVER_TAG << "can't read transferred data from connection." << endl;
                exit(-1);
            }
            if(*sync_id_packet!=(unsigned)Signal::terminator){ //if got valid ID
                //find the pending packet corresponding to resieved ID
                for(auto packet_iterator = pending_packets.begin(); packet_iterator!=pending_packets.end(); packet_iterator++){
                    if(*sync_id_packet == (*packet_iterator)->getID()){
                        cout << SERVER_TAG << "recieved ID: " << *sync_id_packet << endl;
                        (*packet_iterator)->setState(Packet::State::success);
                        Packet::progressDeliverCount(); //add to the total amount of successful deliveries
                        delete *packet_iterator;
                        pending_packets.erase(packet_iterator); //remove from pending list
                        break;
                    }
                }
            }else if(pending_packets.empty()){
                //stop the connection. (all the packets have been either recieved or over-limited)
                //send shutdown signal to the server
                Signal signal = Signal::server_stop;
                if(write(server_tcp_address.getSocket(),&signal,sizeof(int)) < 0){
                    cerr << SERVER_TAG << "can't write TCP." << endl;
                    exit(-1);
                }
                break;
            }
        }
        while(*sync_id_packet!=0); //sync packets stop after null ID recieved.

        cout << endl
             << SERVER_TAG
             << "Done syncing. packets successfuly delivered so far: " << Packet::success_count << "/" << Packet::NUMBER_OF_PACKETS << endl
             << "\t\t\t" << double(Packet::loss_count) / double(Packet::loss_count + Packet::deliver_count) *100 <<"% of the packets got lost in the process so far." << endl
             << "\t\t\t" << Packet::over_limit_count << " packets have exceeded the retry limit(" << Packet::RETRY_LIMIT << ")." << endl
             << endl;

        delete sync_id_packet;
    }
}




int main(int argc, char* argv[])
{
    if(argc==1){
        cerr << "please supply the address/es of the server/s." << endl;
        return -1;
    }

    cout << "starting client for " << argc-1 << " server connections." << endl;

    vector<thread*> client_threads;

    //start all connections
    for(int i=1;i<argc;i++){
        client_threads.push_back(new thread(client,argv[i]));
    }

    //wait for the connections to finish
    for(thread* client_thread : client_threads){
        client_thread->join();
    }


    return 0;

}


