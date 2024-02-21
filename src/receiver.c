#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"

#define MAXSIZE sizeof(struct packet)

void rrecv( unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {

    packet_t incoming_packet;

    char incoming_buf [MAXSIZE];
    packet_t* outgoing_packet;
    header_t* outgoing_header
    size_t recv_len, send_len;

    int sock_fd;
    struct sockaddr_in server_addr, client_addr;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(myUDPport);

    //what does the above do?

    sock_fd = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    //TODO: error checking for sock_fd
    int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    //TODO: error checking for bind


    bool connection_open = true; //us this always true?
    uint32_t expected_sequence = 1;
    uint32_t ack_number
   
    while(connection_open){
        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), MSG_WAITALL, (sock_addr*) &client_addr, sizeof(client_addr));

        if (HAS_FLAGS(incoming_packet->flags)){
            //TODO: handle flags
        }
        else {
            if(incoming_packet->sequence == expected_sequence){
                ack_number = expected_sequence;
                expected_sequence+=1;
                //TODO: write data
                //TODO: check if any enqueued data can be written

            } else {
                ack_number = incoming_packet->sequence;
                //TODO: enqueue this packet somewhere
            }
            outgoing_header = create_header();
            outgoing_packet = create_packet(outgoing_header, NULL);
            //TODO: make outgoing_packet an ack type
            send_len = sendto(sock_fd, outgoing_packet, sizeof(*outgoing_packet), (sock_addr*) &server_addr, sizeof(server_addr) )
        }
    }

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
