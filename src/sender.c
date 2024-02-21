#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    packet_t* pkt = create_packet(0, 1, 2, 3, 4);

    // uint32_t checksum = compute_checksum(pkt);
    //
    // printf("%" PRIu32, checksum);   
    
    // int hostUDPport;
    // unsigned long long int bytesToTransfer;
    // char* hostname = NULL;
    //
    // if (argc != 5) {
    //     fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
    //     exit(1);
    // }
    // hostUDPport = (unsigned short int) atoi(argv[2]);
    // hostname = argv[1];
    // bytesToTransfer = atoll(argv[4]);

    return (EXIT_SUCCESS);
}
