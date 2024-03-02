/**
 * @file udp_file_transfer.c
 * @brief Simple UDP-based file transfer client implementation.
 * @author Connor Johst - cjohst & Aaditya Suri - AadityaSuri
 * @bug No known bugs
 *
 * This file contains the implementation of a simple file transfer client that uses UDP for network communication.
 * It includes functionality to send a file in chunks (packets) to a server, handle acknowledgments (ACKs) for
 * each packet, and perform retransmissions in case of timeouts. The application supports specifying the hostname
 * and port of the receiver, the file to be transferred, and the number of bytes to transfer.
 */

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

#define ACK_TIMEOUT 500 // Timeout for ACKs in milliseconds.

#define min(a, b) ((b) > (a) ? (a) : (b)) // Helper function to find the minimum of two values.


/**
 * Structure to keep track of each packet and its acknowledgment status.
 */
struct packet_ack {
    packet_t packet;
    bool acked;
};


/**
 * Sends a file to a server using UDP.
 * 
 * @param hostname The hostname of the server to send the file to.
 * @param hostUDPport The UDP port number of the server.
 * @param filename The name of the file to send.
 * @param bytes_to_transfer The number of bytes of the file to send.
 */
void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytes_to_transfer) 
{

  int sock_fd;
  struct sockaddr_in server_addr;
  FILE *input_file;
  fd_set readfds;
  struct timeval tv;
  int select_retval;

  

  // open the socket for reading 
  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Socket creation failed: %d\n", sock_fd);
    exit(EXIT_FAILURE);
  }

  // set socket to non-blocking
  fcntl(sock_fd, F_SETFL, O_NONBLOCK);

  // create a server address structure and set the port number
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = hostUDPport;
  server_addr.sin_addr.s_addr = INADDR_ANY;


  // open the file for reading
  if ((input_file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Input file open failed: %d\n");
    exit(EXIT_FAILURE);
  }

  
  uint32_t seq_num = 0;
  header_t header;
  packet_t packet;
  
  unsigned long long int total_bytes_acked = 0;

  // get the total number of bytes in the file
  unsigned long long int file_total_bytes = 0;
  struct stat file_stat;

  if (stat(filename, &file_stat) == -1) {
    fprintf(stderr, "Cannot get file size\n");
    exit(EXIT_FAILURE);
  }
  file_total_bytes = file_stat.st_size;

  int len = sizeof(server_addr);

  // allocate memory for holding all the packets.
  struct packet_ack* packets = (struct packet_ack*) malloc(sizeof(struct packet_ack) * bytes_to_transfer);
  if (packets == NULL) {
    fprintf(stderr, "Cannot allocate memory for packet tracking\n");
    exit(EXIT_FAILURE);
  }
  memset(packets, 0, sizeof(packet_t) * bytes_to_transfer);
  int packet_index = 0;

  int last_packet_acked = -1;

  while(total_bytes_acked < min(file_total_bytes, bytes_to_transfer))   {

    unsigned char buffer[PAYLOAD_SZ];
    size_t bytes_read_in_for_loop = 0;
    
    memset(buffer, 0, PAYLOAD_SZ);
    size_t bytes_to_read_from_file = min(PAYLOAD_SZ, bytes_to_transfer - total_bytes_acked);
    size_t bytes_read_from_file = fread(buffer, sizeof(unsigned char), bytes_to_read_from_file, input_file);

    //create a packet with the data and header
    packet_t packet = create_packet(buffer, 
        create_header(seq_num, 0, bytes_read_from_file, 0));

    // add the packet to the array of packets
    packets[seq_num].packet = packet;
    packets[seq_num].acked = false;
     

    // send the packet and check for errors
    int send_len = sendto(sock_fd, &packet, sizeof(packet.header) + bytes_read_from_file,
          0, (const struct sockaddr*) &server_addr,  len);
    if (send_len < 0) {
      fprintf(stderr, "Send failed: %d\n", send_len);
      exit(EXIT_FAILURE);
    }
      
    printf("SENT PACKET with seq_num: %d\n", packet.header.seq_num);
    seq_num++;
    // packet_index++;


    FD_ZERO(&readfds);
    FD_SET(sock_fd, &readfds);
    tv.tv_sec = ACK_TIMEOUT / 2000;
    tv.tv_usec = (ACK_TIMEOUT % 2000) * 1000;

    select_retval = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
    
    if (select_retval == -1) {
      fprintf(stderr, "Select failed: %d\n", select_retval);
      exit(EXIT_FAILURE);

    } else if (select_retval) {

      // if the select call returned, check if any packets have been acked and increment the total bytes acked
      packet_t ack_packet;
      recvfrom(sock_fd, &ack_packet, sizeof(ack_packet), 
          0, (const struct sockaddr*) &server_addr, &len);

      if (IS_ACK(ack_packet.header.flags)){
        printf("RECEIVED ACK with ack_number: %d\n", ack_packet.header.ack_num);
        packets[ack_packet.header.ack_num].acked = true;
        last_packet_acked = ack_packet.header.ack_num;
        total_bytes_acked += bytes_read_from_file;
      }

    } else {

      // if the select call timed out, retransmit any packets that have not been acked
      printf("TIMEOUT\n");
      printf("LAST PACKET ACKED: %d\n", last_packet_acked);
      for (size_t i = 0 ; i < seq_num; i++) {
        if (!packets[i].acked) {
          packet_t packet = packets[i].packet;
          int send_len = sendto(sock_fd, &packet, sizeof(packet), 0, (const struct sockaddr*) &server_addr,  len);
          printf("PACKET RETRANSMITTED with seq_num: %d\n", packet.header.seq_num);
          if (send_len < 0) {
            fprintf(stderr, "Send failed: %d\n", send_len);
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  
  }

  printf("%d packets sent\n", seq_num);


  header = create_header(0,0,0, FIN_FLAG);
  packet = create_packet(NULL, header);
  sendto(sock_fd, &packet, sizeof(packet), 0,
    (const struct sockaddr*) &server_addr,  sizeof(server_addr));
  printf("SENT FIN\n");

  free(packets);

  //close socket
  close(sock_fd);

}

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    
    int host_udp_port;
    unsigned long long int bytes_to_transfer;
    char* hostname = NULL;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    host_udp_port = (unsigned short int) atoi(argv[2]);
    hostname = argv[1];
    bytes_to_transfer = atoll(argv[4]);

    rsend(hostname, host_udp_port, argv[3], bytes_to_transfer);

    return (EXIT_SUCCESS);
}
