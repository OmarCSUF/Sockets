all: server client

Address.o: Address.h Address.cpp
	g++ -Wall -std=c++1z Address.cpp -c
	
Packet.o: Packet.h Packet.cpp
	g++ -Wall -std=c++1z Packet.cpp -c
	
common: Address.o Packet.o common.h
	
server: common server.cpp
	g++ -Wall -std=c++1z -pthread server.cpp Address.o Packet.o -o server
	
client: common client.cpp
	g++ -Wall -std=c++1z -pthread client.cpp Address.o Packet.o -o client

clean:
	rm -f *.o && rm -f *.stackdump && rm -f *.exe && rm -f server && rm -f client && rm -f sockets.zip
	
zip:
	zip sockets makefile readme.txt common.h server.cpp client.cpp Address.h Address.cpp Packet.h Packet.cpp
	
