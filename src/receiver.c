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
#include <time.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"
#include "priorityqueue.h"

size_t writeWithRate(char data[], int data_len, unsigned long long int write_rate, size_t total_bytes_written, time_t start_time, FILE* outfile) {
    size_t bytes_written = 0;
    time_t current_time;
    time(&current_time);

    if (write_rate == 0) {
        for (size_t i = 0; i < data_len; i++){
            // printf("%c", data[i]);
            bytes_written += fprintf(outfile, "%c", data[i]);
            // printf("%c", data[i]);
        }
        //TODO: handle error if bytes written is not correct value
        return bytes_written;
    }

    double elapsed_seconds = difftime(current_time, start_time);
    double write_rate_if_all_written = ((double) total_bytes_written + (double)data_len) / elapsed_seconds;


    while (write_rate_if_all_written > (double) write_rate) { 
        sleep(0.25);
        time(&current_time);
        elapsed_seconds = difftime(current_time, start_time);
        write_rate_if_all_written = ((double) total_bytes_written + (double)data_len) / elapsed_seconds;
    }

    for (size_t i = 0; i < data_len; i++){
            bytes_written += fprintf(outfile, "%c", data[i]);
        }
    //TODO: handle error if bytes written is not correct value
    return bytes_written;
}


void rrecv( unsigned short int udp_port, 
            char* destination_file, 
            unsigned long long int write_rate) {

    time_t start_time;
    time(&start_time);

    FILE *outfile = fopen(destination_file, "w");

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

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

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

    int len = sizeof(client_addr);

    while(connection_open){
        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), 0, 
         (const struct sock_addr*) &client_addr, &len);
        
        // if (recv_len < 0) {
        //     fprintf(stderr, "Socket receive failed: %d\n", recv_len);
        //     close(sock_fd);
        //     exit(EXIT_FAILURE);
        // }

        // printf("INCOMING SEQUENCE:%d\n", incoming_packet.header.seq_num);

        if (IS_FIN(incoming_packet.header.flags)){
            //handle flags, send FIN ACK?
            connection_open = false;
            break;

        }
        else {
            if(incoming_packet.header.seq_num < expected_sequence){
                //TODO: discard packet
                printf("DISCARDING  packet with seq_num: %d\n", incoming_packet.header.seq_num);
                memset(&incoming_packet, 0, sizeof(incoming_packet));
            } else if (incoming_packet.header.seq_num > expected_sequence) {
                ack_number = incoming_packet.header.seq_num;
                //enqueue packet with priority seq_num to be written later
                printf("ENQUEUING  packet with seq_num: %d\n", incoming_packet.header.seq_num);
                enqueue(packet_queue, incoming_packet.header.seq_num, incoming_packet.data);
            } else {
                //write packet
                printf("WRITING packet with seq_num: %d\n", incoming_packet.header.seq_num);
                total_bytes_written += writeWithRate(incoming_packet.data, sizeof(incoming_packet.data), write_rate, total_bytes_written, start_time, outfile);
                //TODO: handle errors with return value of writeWithRate
                ack_number = expected_sequence;
                expected_sequence+=1;

            }
            //check if ANY enqueued data can be written and write it
            while(peak(packet_queue) == expected_sequence) {
                printf("PEAKING AT QUEUE\n");
                QueueNode dequeued_node = dequeue(packet_queue);
                printf("WRITING QUEUED  packet with seq_num: %d\n", dequeued_node.priority);
                total_bytes_written += writeWithRate(dequeued_node.data, sizeof(dequeued_node.data), write_rate, total_bytes_written, start_time, outfile);
                //TODO: handle errors with return value of writeWithRate
            }

            outgoing_header = create_header(0, ack_number, 0, ACK_FLAG);
            outgoing_packet = create_packet(NULL, outgoing_header);
            send_len = sendto(sock_fd, &outgoing_packet, sizeof(outgoing_packet), 0, (const struct sock_addr*) &client_addr, len);
            //TODO: handle errors when ack packet not sent correctly
        }
        
    }
    fclose(outfile);
    printf("DIFFERENCE BETWEEN START AND END TIME: %f", difftime(time(NULL), start_time));
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
