#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>

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

struct packet_ack {
    packet_t packet;
    bool acked;
};


void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{
  int sockfd;
  struct sockaddr_in server_addr;
  FILE *file;
  fd_set readfds;
  struct timeval tv;
  int retval;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // set socket to non-blocking
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

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

  unsigned long long int fileTotalBytes = 0;
  struct stat fileStat;
  if (stat(filename, &fileStat) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  fileTotalBytes = fileStat.st_size;

  int len = sizeof(server_addr);

  int packet_num = 0;
  struct packet_ack* packets = (struct packet_ack*) malloc(sizeof(struct packet_ack) * bytesToTransfer);
  if (packets == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  memset(packets, 0, sizeof(packet_t) * bytesToTransfer);

  //this while loop will seg fault if bytesToTransfer is > actual file size
  while(totalSent < min(fileTotalBytes, bytesToTransfer))   {

      unsigned char buffer[PAYLOAD_SZ];
      memset(buffer, 0, PAYLOAD_SZ);
      size_t bytesToRead = min(PAYLOAD_SZ, bytesToTransfer - totalSent);
      size_t bytesRead = fread(buffer, sizeof(unsigned char), bytesToRead, file);

      // header_t header = create_header(seq_num++, 0, bytesRead, 0);
      packet_t packet = create_packet(buffer, 
          create_header(seq_num, 0, bytesRead, 0));

      packets[seq_num].packet = packet;
      packets[seq_num].acked = false;
      seq_num++;

      srand(time(NULL));
      int rand_num = rand() % 100;

      if (rand_num < 50) {
        int send_len = sendto(sockfd, &packet, sizeof(packet.header) + bytesRead,
            0, (const struct sockaddr*) &server_addr,  len);
      } else {
        printf("PACKET with seq_num: %d NOT sent\n", packet.header.seq_num);
      }
      packet_num++;
      

      FD_ZERO(&readfds);
      FD_SET(sockfd, &readfds);
      tv.tv_sec = ACK_TIMEOUT / 2000;
      tv.tv_usec = (ACK_TIMEOUT % 2000) * 1000;
      retval = select(sockfd + 1, &readfds, NULL, NULL, &tv);

      if (retval == -1) {
        perror("select");
        exit(EXIT_FAILURE);
      } else if (retval) {
        packet_t ack_packet;
        recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 
            0, (const struct sockaddr*) &server_addr, &len);

        if (IS_ACK(ack_packet.header.flags)){
          printf("RECEIVED ACK with ack_number: %d\n", ack_packet.header.ack_num);
          packets[ack_packet.header.seq_num].acked = true;
        }

      } else {
        printf("TIMEOUT\n");
        for (size_t i = 0; i < packet_num; i++) {
          if (!packets[i].acked) {
            packet_t packet = packets[i].packet;
            int send_len = sendto(sockfd, &packet, sizeof(packet), 0, (const struct sockaddr*) &server_addr,  len);
            printf("PACKET RETRANSMITTED with seq_num: %d\n", packet.header.seq_num);
            if (send_len == -1) {
              perror("sendto");
              exit(EXIT_FAILURE);
            }
          }
        }

      }

      totalSent += bytesRead;
  }

  printf("%d packets sent\n", packet_num);


  header = create_header(0,0,0, FIN_FLAG);
  packet = create_packet(NULL, header);
  sendto(sockfd, &packet, sizeof(packet), 0,
    (const struct sockaddr*) &server_addr,  sizeof(server_addr));
  printf("SENT FIN\n");

  free(packets);

  //close socket
  close(sockfd);

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    
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

    rsend(hostname, hostUDPport, argv[3], bytesToTransfer);

    return (EXIT_SUCCESS);
}
