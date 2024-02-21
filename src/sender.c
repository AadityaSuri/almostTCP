#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
  int sockfd;
  struct sockaddr_in server_addr;
  FILE *file;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(hostUDPport);
  server_addr.sin_addr.s_addr = INADDR_ANY;


  if ((file = fopen(filename, "r")) == NULL) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }

  
  uint32_t seq_num = 0;
  unsigned long long int totalSent = 0;
  size_t bytesRead;
  header_t header;
  packet_t packet;
  unsigned char buffer[64];

  //this while loop will seg fault if bytesToTransfer is > actual file size
  while(totalSent < bytesToTransfer)   { 
    memset(buffer, 0, 64);
    size_t bytesRead = fread(buffer, 1, 64, file);

    header_t header = create_header(seq_num++, 0, 64, 0);
    packet_t packet = create_packet(buffer, header);

    sendto(sockfd, &packet, sizeof(packet), 0,
    (const struct sockaddr*) &server_addr,  sizeof(server_addr));
    totalSent += packet.header.length;
  }
  header = create_header(0,0,0, FIN_FLAG);
  packet = create_packet(NULL, header);
  sendto(sockfd, &packet, sizeof(packet), 0,
    (const struct sockaddr*) &server_addr,  sizeof(server_addr));
  printf("SENT FIN\n");


  // header_t header = create_header(0, 1, 2, 3, 4);
  // unsigned char mssg[64] = "hello from sender\n";
  // packet_t pkt = create_packet(mssg, header);


  // while (true) {
  //   sendto(sockfd, &pkt, sizeof(pkt),
	// 	0, (const struct sockaddr *) &server_addr,
	// 		sizeof(server_addr));
	//   printf("Hello message sent.\n");
  // }
  

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    // packet_t* pkt = create_packet(0, 1, 2, 3, 4);

    // uint32_t checksum = compute_checksum(pkt);
    //
    // printf("%" PRIu32, checksum);   
    
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char* hostname = NULL;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    hostUDPport = (unsigned short int) atoi(argv[2]);
    hostname = argv[1];
    bytesToTransfer = atoll(argv[4]);

    // rsend("localhost", 8080, "testfile.txt", 100);
    rsend(hostname, hostUDPport, "potter.txt", bytesToTransfer);

    return (EXIT_SUCCESS);
}
