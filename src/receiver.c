#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"


void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {

    bool connection_open = false;
    int sock_fd = initializeSocket(myUDPport);

    connection_open = handshake(sock_fd);

    while(connection_open){
        """
        CORE LOOP
        read from socket into incoming buf
        compute checksum
        if checksum correct:
            check flags:
                if no flags:
                    check seq#:
                        if seq# correct:
                            write data
                            check if queued data can be written
                            send ack
                        else:
                            hold data in queue
                            await new data
                if flags:
                    start end connection protocol
                    connection_open = false
        else:
            discard data
        """
    }            
}

bool handshake(int sock_fd){
    //conduct tcp handshake
}

int initializeSocket(unsigned short int myUDPport){

    int sock_fd;
    struct sockaddr_in server_addr;
    bool connection_open = false;

    sock_fd = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(myUDPport);

    //error checking for sock_fd
    int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    //error checking for bind

    return sock_fd;
}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);
}
