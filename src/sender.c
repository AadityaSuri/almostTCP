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
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"

#define ACK_TIMEOUT 150 // Timeout for ACKs in milliseconds.
#define FIN_ACK_WAIT 100 // Time to wait for FIN ACKs in microseconds.
#define MAX_FIN_SENT 10 // Maximum number of times to send FIN packets before giving up.

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
  bytes_to_transfer = min (file_total_bytes, bytes_to_transfer);

  int len = sizeof(server_addr);

  // allocate memory for holding all the packets.
  struct packet_ack* packets = (struct packet_ack*) malloc(sizeof(struct packet_ack) * ((bytes_to_transfer / PAYLOAD_SZ) + 1));
  if (packets == NULL) {
    fprintf(stderr, "Cannot allocate memory for packet tracking\n");
    exit(EXIT_FAILURE);
  } 
  memset(packets, 0, sizeof(packet_t) * ((bytes_to_transfer / PAYLOAD_SZ) + 1));
  long long int packet_index = 0;

  long long int total_bytes_read = 0;

  while(total_bytes_acked < bytes_to_transfer)   {

    unsigned char buffer[PAYLOAD_SZ];
    memset(buffer, 0, PAYLOAD_SZ);
    if (total_bytes_read < bytes_to_transfer) {

      // read the next chunk of data from the file
      size_t bytes_read_from_file = fread(buffer, sizeof(unsigned char), 
          min(PAYLOAD_SZ, bytes_to_transfer - total_bytes_read), input_file);
      total_bytes_read += bytes_read_from_file; 

    
      //create a packet with the data and header
      packet_t packet = create_packet(buffer, 
          create_header(seq_num, 0, bytes_read_from_file, 0));

      // add the packet to the array of packets
      packets[packet_index].packet = packet;
      packets[packet_index].acked = false;
      
      // send the packet and check for errors
      int send_len = sendto(sock_fd, &packet, sizeof(packet.header) + bytes_read_from_file,
            0, (const struct sockaddr*) &server_addr,  len);
      if (send_len < 0) {
        fprintf(stderr, "Send failed: %d\n", send_len);
        exit(EXIT_FAILURE);
      }
        
      seq_num++;
      packet_index++;
    }


    FD_ZERO(&readfds);
    FD_SET(sock_fd, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = ACK_TIMEOUT * 1000;

    // monitor the socket for incoming packets 
    select_retval = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
    
    if (select_retval == -1) {
      fprintf(stderr, "Select failed: %d\n", select_retval);
      exit(EXIT_FAILURE);

    } else if (select_retval) {

      // if the select call returned, check if any packets have been acked and increment the total bytes acked
      packet_t ack_packet;
      recvfrom(sock_fd, &ack_packet, sizeof(packet_t), 
          0, (const struct sockaddr*) &server_addr, &len);

      packets[ack_packet.header.ack_num].acked = true;
      total_bytes_acked += ack_packet.header.length;
 
    } else {

      // if the select call timed out, loop through all packets and retransmit any packets that have not been acked
      for (unsigned long long int i = 0 ; i < packet_index; i++) {
        if (!packets[i].acked) {
          packet_t retransmit_packet = packets[i].packet;
          int send_len = sendto(sock_fd, &retransmit_packet, sizeof(packet_t), 0, (const struct sockaddr*) &server_addr,  len);
          if (send_len < 0) {
            fprintf(stderr, "Send failed: %d\n", send_len);
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  
  }


  packet_t fin_packet, fin_ack_packet;
  bool fin_ack_flag = false;

  int fin_sent = 0;
  while (!fin_ack_flag && fin_sent < MAX_FIN_SENT) {

    // send FIN packet
    fin_packet = create_packet(NULL, 
        create_header(0,0,-1, FIN_FLAG));
    sendto(sock_fd, &fin_packet, sizeof(packet_t), 0,
      (const struct sockaddr*) &server_addr,  len);

    usleep(FIN_ACK_WAIT);
    
    // listen for FIN ACK
    if (recvfrom(sock_fd, &fin_ack_packet, sizeof(packet_t), 
        0, (const struct sockaddr*) &server_addr, &len) < 0) {
    }

    fin_ack_flag = IS_FIN(fin_ack_packet.header.flags) && IS_ACK(fin_ack_packet.header.flags);
    fin_sent++;
  }

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
