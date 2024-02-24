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
#include "priorityqueue.h"

void rrecv( unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {

    FILE *outfile = fopen(destinationFile, "a");

    packet_t incoming_packet, outgoing_packet;
    header_t outgoing_header;
    PriorityQueue packet_queue;
    
    size_t recv_len, send_len;
    struct sockaddr_in server_addr, client_addr;

    int sock_fd = socket(
        AF_INET,
        SOCK_DGRAM,
        0
    );

    if (sock_fd < 0) {
        fprintf(stderr, "Socket creation failed: %d\n", sock_fd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(myUDPport);

    int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    if (bind_code < 0) {
        fprintf(stderr, "Socket bind failed: %d\n", bind_code);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    bool connection_open = true; //is this always true?
    uint32_t expected_sequence = 0;
    uint32_t ack_number;

    while(connection_open){
        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), 0, (const struct sock_addr*) &client_addr, sizeof(client_addr));
        
        if (recv_len < 0) {
            fprintf(stderr, "Socket receive failed: %d\n", recv_len);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        if (IS_FIN(incoming_packet.header.flags)){
            //handle flags, send FIN ACK?
            connection_open = false;
            break;

        }
        else {
            if(incoming_packet.header.seq_num < expected_sequence){
                //TODO: discard packet
            } else if (incoming_packet.header.seq_num > expected_sequence) {
                ack_number = incoming_packet.header.seq_num;
                //TODO: enqueue this packet somewhere
            } else {
                ack_number = expected_sequence;
                expected_sequence+=1;
                for (int i = 0; i < 64; i++){
                    fprintf(outfile, "%c", incoming_packet.data[i]);
                }  
            }
            // TODO: check if enqueued data can be written


            outgoing_header = create_header(expected_sequence, ack_number, 0, ACK_FLAG);
            outgoing_packet = create_packet(NULL, outgoing_header);
            send_len = sendto(sock_fd, &outgoing_packet, sizeof(outgoing_packet), 0, (const struct sock_addr*) &server_addr, sizeof(server_addr));
        }
        
    }
    fclose(outfile);
    exit(EXIT_SUCCESS);
}            


int main(int argc, char** argv) {

    unsigned short int udpPort;
    char* destinationFile;
    unsigned long long int writeRate;

    if (argc != 4) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write writerate\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);
    destinationFile  = argv[2];
    writeRate = (unsigned long long int) atoi(argv[3]);

    rrecv(udpPort, destinationFile, writeRate);
}
