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

// #define MAXSIZE sizeof(struct packet)
//
// void rrecv(unsigned short int myUDPport, 
//             char* destinationFile, 
//             unsigned long long int writeRate) {
//
//     packet_t incoming_packet;
//     packet_t outgoing_packet;
//
//     bool connection_open = false;
//     bool checksum = true;
//
//     uint32_t expected_sequence = 1;
//
//     int sock_fd = initializeSocket(myUDPport);
//     connection_open = handshake(sock_fd);
//
//     while(connection_open){
//         size_t recv_len = recvfrom(sock_fd, incoming_packet, sizeof(incoming_packet), MSG_WAITALL);
//         //some error handling on if recv_len is correct value?
//         //compute checksum here, set value of checksum boolean
//         if (checksum){
//             if (HAS_FLAGS(incoming_packet->flags)){
//                 //handle flags
//             }
//             else {
//                 if(incoming_packet->sequence == expected_sequence){
//                     //write data
//                     //check if any enqueued data can be written
//
//                 } else {
//                     //enqueue this packet somewhere
//                 }
//                 //create outgoing packet
//
//                 //send ack
//
//             }
//         }
//         else {
//             //error handling for invalid checksum 
//             //likely discard
//         }
//
//
//     }            
// }
//
//
// bool handshake(int sock_fd){
//     //conduct tcp handshake
// }
//
// int initializeSocket(unsigned short int myUDPport){
//
//     int sock_fd;
//     struct sockaddr_in server_addr;
//     bool connection_open = false;
//
//     sock_fd = socket(
//         AF_INET,
//         SOCK_DGRAM,
//         IPPROTO_UDP
//     );
//
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     server_addr.sin_port = htons(myUDPport);
//
//     //error checking for sock_fd
//     int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
//     //error checking for bind
//
//     return sock_fd;
// }

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
