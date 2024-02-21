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

    FILE *outfile = fopen("output.txt", "a");

    packet_t incoming_packet, outgoing_packet;
    header_t outgoing_header;
    
    size_t recv_len, send_len;
    int sock_fd;
    struct sockaddr_in server_addr, client_addr;

    sock_fd = socket(
        AF_INET,
        SOCK_DGRAM,
        0
    );

    //TODO: error checking for sock_fd

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(myUDPport);

    int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    //TODO: error checking for bind

    bool connection_open = true; //is this always true?
    uint32_t expected_sequence = 0;
    uint32_t ack_number;

    //test code
    // while (true) {
    //         recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), MSG_WAITALL, (const struct sockaddr*) &client_addr, sizeof(client_addr));
    //         if (recv_len > 0){
    //             printf("%d", incoming_packet.header.ack_num);
    //             break;
    //         }
    // }

    while(connection_open){
        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), 0, (const struct sock_addr*) &client_addr, sizeof(client_addr));
        //TODO: error checking for recv_len
        if (HAS_FLAGS(incoming_packet.header.flags)){
            //handle flags
            connection_open = false;
        }
        else {
            if(incoming_packet.header.seq_num == expected_sequence){
                ack_number = expected_sequence;
                expected_sequence+=1;
                fprintf(outfile, "%s", incoming_packet.data);
                //TODO: check if any enqueued data can be written

            } else {
                ack_number = incoming_packet.header.seq_num;
                //TODO: enqueue this packet somewhere
            }
            outgoing_header = create_header(expected_sequence, ack_number, 0, ACK_FLAG);
            outgoing_packet = create_packet(NULL, outgoing_header);
            send_len = sendto(sock_fd, &outgoing_packet, sizeof(outgoing_packet), 0, (const struct sock_addr*) &server_addr, sizeof(server_addr));
        }
        fclose(outfile);
    }
}            


int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    // unsigned short int udpPort;

    // if (argc != 3) {
    //     fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
    //     exit(1);
    // }

    // udpPort = (unsigned short int) atoi(argv[1]);

    rrecv(8080, 0, 0);

}
