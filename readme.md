usage:
(1) run "make" to compile the whole project
(2) run "./server" on all the machines you want to test on
(3) run "./client <IP1> <IP2> ... <IPn>" to run the client on each server. eg: "./client localhost 192.168.0.2 192.168.0.3"



quick technical overview:
	client sends fixed length UDP packets to each server, at fixed probability of loss (defined in Packet.h alongside other related constants). 
	once UDP packet arrives to a server, the server stores it's ID in a queue, that gets flushed back to the client by TCP later,
	to indicate recipient. the flush is triggered by the client sending a special TCP signal.
	the TCP listener on the server runs alongsize UDP listener in parallel (in different threads).
	once to client gets aknowledged of the successful arrival of UDP packet to the server (after the flush), it removes the packet from
	the "to-send queue". 
	the client utilizes multithreading for simultaneous run over single or multiple servers (amount limited by hardware only), which
	never interfer one with another. 
	once a packet exceeds maximum retry limit, it get's marked as failed and getting removed from the queue.
	once all the packets are either sent successfuly or exceeded retry limit, the client tells the server to terminate with success.

implementation overview:
	the tool is GCC C++17(c++1z) compiler, should compile on any C++11 compatible toolchain.
	i've created a class wrapper around some C low level (standard) network utilization, providing easier and more intuitive functionality
	with less code repitition, and better structure. this mainly includes the "Address" and the "Packet" classes.  
	"server.cpp" and "client.cpp" are source files for the two programs (server and client), both are using "common.h" to wrap the inclusion
	and ensure compatible headers in need of patch. 
	the code is well commented and verbose namings are highly adopted over short undescribable ones 
		-> shouldn't be any big problem to read and understand. 
	

output:
	per server:
		* amount of successfully recieved packets on the server side
		* percentage of packet loss (including retries)
		* amount of packets that excceeded the retry limit

default values:
	port = 1337
	number of packets per server = 10,000
    	packet length = 1,000 (bytes)
    	loss probability = 66%
    	send retry limit = 3

	