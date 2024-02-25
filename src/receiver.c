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

size_t writeWithRate(char data[], unsigned long long int write_rate, size_t total_bytes_written, time_t start_time, FILE* outfile) {
    size_t data_length = sizeof(data);
    size_t bytes_written = 0;

    if (write_rate == 0) {
        for (size_t i = 0; i < data_length; i++){
            bytes_written += fputc(data[i], outfile);
            // printf("%c", data[i]);
        }
        //TODO: handle error if bytes written is not correct value
        return bytes_written;
    }

    double elapsed_deconds = difftime(time(NULL), start_time);
    double write_rate_if_all_written = (total_bytes_written + data_length) / elapsed_deconds;

    while (write_rate_if_all_written > write_rate) { 
        sleep(0.25);
        elapsed_deconds = difftime(time(NULL), start_time);
        write_rate_if_all_written = (total_bytes_written + data_length) / elapsed_deconds;
    }

    for (size_t i = 0; i < data_length; i++){
            bytes_written += fputc(data[i], outfile);
        }
    //TODO: handle error if bytes written is not correct value
    return bytes_written;
}


void rrecv( unsigned short int udp_port, 
            char* destination_file, 
            unsigned long long int write_rate) {

    time_t start_time = time(NULL);

    FILE *outfile = fopen(destination_file, "a");

    packet_t incoming_packet, outgoing_packet;
    header_t outgoing_header;
    PriorityQueue* packet_queue = createPriorityQueue();
    
    int recv_len, send_len;
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
    server_addr.sin_port = htons(udp_port);

    int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    if (bind_code < 0) {
        fprintf(stderr, "Socket bind failed: %d\n", bind_code);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    bool connection_open = true; //is this always true?
    uint32_t expected_sequence = 0;
    uint32_t ack_number;
    size_t total_bytes_written = 0;

    while(connection_open){
        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), 0, (const struct sock_addr*) &client_addr, sizeof(client_addr));
        
        // if (recv_len < 0) {
        //     fprintf(stderr, "Socket receive failed: %d\n", recv_len);
        //     close(sock_fd);
        //     exit(EXIT_FAILURE);
        // }

        if (IS_FIN(incoming_packet.header.flags)){
            //handle flags, send FIN ACK?
            connection_open = false;
            break;

        }
        else {
            if(incoming_packet.header.seq_num < expected_sequence){
                //TODO: discard packet
                printf("%d, %d", incoming_packet.header.seq_num);
                printf("DISCARDING PACKET");
                memset(&incoming_packet, 0, sizeof(incoming_packet));
            } else if (incoming_packet.header.seq_num > expected_sequence) {
                ack_number = incoming_packet.header.seq_num;
                //enqueue packet with priority seq_num to be written later
                printf("ENQUEUING PACKET");
                enqueue(packet_queue, incoming_packet.header.seq_num, incoming_packet.data);
            } else {
                //write packet
                // for (int i = 0; i < sizeof(incoming_packet.data); i++){printf("%c", incoming_packet.data[i]);} 
                total_bytes_written =+ writeWithRate(incoming_packet.data, write_rate, total_bytes_written, start_time, outfile);
                ack_number = expected_sequence;
                expected_sequence+=1;

            }
            //check if ANY enqueued data can be written and write it
            while(peak(packet_queue) == expected_sequence) {
                QueueNode dequeued_node = dequeue(packet_queue);
                total_bytes_written =+ writeWithRate(dequeued_node.data, write_rate, total_bytes_written, start_time, outfile);
                //TODO: handle errors with return value of writeWithRate
            }

            outgoing_header = create_header(0, ack_number, 0, ACK_FLAG);
            outgoing_packet = create_packet(NULL, outgoing_header);
            send_len = sendto(sock_fd, &outgoing_packet, sizeof(outgoing_packet), 0, (const struct sock_addr*) &server_addr, sizeof(server_addr));
            //TODO: handle errors when ack packet not sent correctly
        }
        
    }
    fclose(outfile);
    exit(EXIT_SUCCESS);
}            


int main(int argc, char** argv) {

    unsigned short int udp_port;
    char* destination_file;
    unsigned long long int write_rate;

    if (argc != 4) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write writerate\n\n", argv[0]);
        exit(1);
    }

    udp_port = (unsigned short int) atoi(argv[1]);
    destination_file  = argv[2];
    write_rate = (unsigned long long int) atoi(argv[3]);

    rrecv(udp_port, destination_file, write_rate);
}
