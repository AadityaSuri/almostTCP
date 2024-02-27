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
#include <sys/stat.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"
#include "dynamic_list.h"

#define ACK_TIMEOUT 1000
#define MAX_RETRIES 5
#define MAX_CONSECUTIVE_PACKETS 10

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((b) > (a) ? (a) : (b))

struct packet_retry {
    packet_t packet;
    int retries;
};


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
  // unsigned char buffer[64];

  unsigned long long int fileTotalBytes = 0;
  struct stat fileStat;
  if (stat(filename, &fileStat) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  fileTotalBytes = fileStat.st_size;

  int len = sizeof(server_addr);

  //this while loop will seg fault if bytesToTransfer is > actual file size
  while(totalSent < min(fileTotalBytes, bytesToTransfer))   {

    // read 10 consecutive packets from file and send them
    for (size_t i = 0; i < MAX_CONSECUTIVE_PACKETS; i++) {
        unsigned char buffer[PAYLOAD_SZ];
        memset(buffer, 0, PAYLOAD_SZ);
        size_t bytesToRead = min(PAYLOAD_SZ, bytesToTransfer - totalSent);
        size_t bytesRead = fread(buffer, sizeof(unsigned char), bytesToRead, file);

        // header_t header = create_header(seq_num++, 0, bytesRead, 0);
        packet_t packet = create_packet(buffer, 
            create_header(seq_num++, 0, bytesRead, 0));

        // int bytesToSend = sizeof(packet.header) + min(bytesRead, bytesToTransfer - totalSent);

        int send_len = sendto(sockfd, &packet, sizeof(packet.header) + bytesRead,
            0, (const struct sockaddr*) &server_addr,  len);

        totalSent += bytesRead;

    }

    // unsigned char buffer[PAYLOAD_SZ];
    // memset(buffer, 0, PAYLOAD_SZ);
    // int bytesToRead = min(PAYLOAD_SZ, bytesToTransfer - totalSent);
    // size_t bytesRead = fread(buffer, sizeof(unsigned char), bytesToRead, file);
    // //prtinf buffer
    // printf("%s\n", buffer);
    
    // // printf("%s\n", buffer);

    // // header_t header = create_header(seq_num++, 0, bytesRead, 0);
    // packet_t packet = create_packet(buffer, 
    //     create_header(seq_num++, 0, bytesRead, 0));
    
    // // printf("%s\n", packet.data);

    // // int bytesToSend = min(bytesRead, bytesToTransfer - totalSent);

    // int send_len = sendto(sockfd, &packet, sizeof(packet.header) + bytesRead,
    //     0, (const struct sockaddr*) &server_addr,  len);

    // // append_packet(packet_list, &packet);
    // totalSent += bytesRead;
    // printf("SENT %d\n", totalSent);

    /*
    wait for ack from all 10 packets
    */

    // while (true) {
    //     packet_t ack_packet;
    //     recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 
    //         0, (const struct sockaddr*) &server_addr, &len);
    // }
    // totalSent += packet.header.length;
    // int debug = 0;
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
    rsend(hostname, hostUDPport, argv[3], bytesToTransfer);

    return (EXIT_SUCCESS);
}
