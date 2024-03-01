/**
 * @file main.c
 * @brief Implements a receiver program for writing data with specified rate.
 * @author Connor Johst - cjohst & Aaditya Suri - AadityaSuri
 * @bug Program terminates when any socket reads encounter an error 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <time.h>
#include <sys/time.h>   
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>
#include <errno.h>

#include "packet.h"
#include "priorityqueue.h"

/**
 * @brief Writes data to a file with a specified rate.
 * 
 * This function writes data to the specified file pointer while maintaining a desired write rate. 
 * If the write rate is set to 0, data is written as fast as possible.
 * 
 * @param data The array of characters containing the data to be written.
 * @param data_len The length of the data array.
 * @param write_rate The desired write rate in bytes per second. If set to 0, writes data as fast as possible.
 * @param total_bytes_written The total number of bytes already written to the file.
 * @param start_time The start time of the writing process.
 * @param outfile The file pointer to write the data to.
 * @return The total number of bytes successfully written to the file.
 */

size_t writeWithRate(char data[], int data_len, unsigned long long int write_rate, size_t total_bytes_written, time_t start_time, FILE* outfile) {
    size_t bytes_written = 0;
    time_t current_time;
    time(&current_time);

    if (write_rate == 0) {
        // for (size_t i = 0; i < data_len; i++){
        //     // printf("%c", data[i]);
        //     bytes_written += fprintf(outfile, "%c", data[i]);\

        //     // printf("%c", data[i]);
        // }
        bytes_written = fwrite(data, sizeof(char), data_len, outfile);
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

/**
 * @brief Receives data packets over UDP, writes them to a file, and sends acknowledgments.
 * 
 * This function acts as a receiver for UDP packets, writing received data to a specified file 
 * while also sending acknowledgments back to the sender. It maintains a desired write rate if specified.
 * 
 * @param udp_port The UDP port to listen for incoming packets.
 * @param destination_file The file to write the received data to.
 * @param write_rate The desired write rate in bytes per second. If set to 0, writes data as fast as possible.
 */
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
    server_addr.sin_port = udp_port;

    int bind_code = bind(sock_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    if (bind_code < 0) {
        fprintf(stderr, "Socket bind failed: %d\n", bind_code);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    bool connection_open = true;

    uint32_t expected_sequence = 0;
    uint32_t ack_number;
    size_t total_bytes_written = 0;

    int len = sizeof(client_addr);

    //debugging 
    bool first_packet_received = false;

    struct timeval tic, toc;

    bool packet_queue_empty = true;

    while(connection_open){

        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), 0, (const struct sock_addr*) &client_addr, &len);
        if (recv_len == -1) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        } 

        printf("RECEIVED PACKET with seq_num: %d\n", incoming_packet.header.seq_num);

        //debugging
        if (!first_packet_received) {
            gettimeofday(&tic, NULL);
            first_packet_received = true;
        }

        if (IS_FIN(incoming_packet.header.flags)){
            // measure end time here

            gettimeofday(&toc, NULL);
            double elapsed_time = (double) (toc.tv_sec - tic.tv_sec) * 1000.0;
            elapsed_time += (double) (toc.tv_usec - tic.tv_usec) / 1000.0;
            printf("elapsed time: %.3f\n", elapsed_time);

            connection_open = false;
            break;        
        }
        else {
            if(incoming_packet.header.seq_num < expected_sequence){
                //TODO: discard packet
                printf("DISCARDING  packet with seq_num: %d\n", incoming_packet.header.seq_num);
                memset(&incoming_packet, 0, sizeof(incoming_packet));
                continue;
            } else if (incoming_packet.header.seq_num > expected_sequence) {
                ack_number = incoming_packet.header.seq_num;
                //enqueue packet with priority seq_num to be written later
                printf("%d %d\n", ack_number, expected_sequence);
                printf("ENQUEUING  packet with seq_num: %d\n", incoming_packet.header.seq_num);
                enqueue(packet_queue, incoming_packet.header.seq_num, incoming_packet.data, incoming_packet.header.length);
            } else {
                //write packet
                printf("WRITING packet with seq_num: %d\n", incoming_packet.header.seq_num);
                total_bytes_written += writeWithRate(incoming_packet.data, incoming_packet.header.length, write_rate, total_bytes_written, start_time, outfile);
                ack_number = expected_sequence;
                expected_sequence+=1;

            }
            //check if ANY enqueued data can be written and write it
            while(peak(packet_queue) <= expected_sequence) {
                printf("PEAKING AT QUEUE peek: %d\n", peak(packet_queue));
                if (peak(packet_queue) < expected_sequence){
                    //ensure we never write the same packet twice ()
                    QueueNode dequeued_node = dequeue(packet_queue);
                    continue;
                }
                QueueNode dequeued_node = dequeue(packet_queue);
                printf("WRITING QUEUED  packet with seq_num: %d\n", dequeued_node.priority);
                total_bytes_written += writeWithRate(dequeued_node.data, sizeof(dequeued_node.data), write_rate, total_bytes_written, start_time, outfile);
                expected_sequence += 1;
            }

            // packet_queue_empty = packet_queue->size == 0;

            outgoing_header = create_header(0, ack_number, 0, ACK_FLAG);
            outgoing_packet = create_packet(incoming_packet.data, outgoing_header);
            send_len = sendto(sock_fd, &outgoing_packet, sizeof(outgoing_packet), 0, (const struct sock_addr*) &client_addr, len);
            if (send_len < 0) {
                fprintf(stderr, "Ack send failed: %d\n", send_len);
                exit(EXIT_FAILURE);
            }
            printf("SENT ACK with ack_number: %d\n", ack_number);
        }
    }

    printf("%d START\n", packet_queue->size);
    
    //check if ANY enqueued data can be written and write it
            while(peak(packet_queue) <= expected_sequence) {
                printf("PEAKING AT QUEUE\n");
                if (peak(packet_queue) < expected_sequence){
                    //ensure we never write the same packet twice ()
                    QueueNode dequeued_node = dequeue(packet_queue);
                    continue;
                }
                QueueNode dequeued_node = dequeue(packet_queue);
                printf("WRITING QUEUED  packet with seq_num: %d\n", dequeued_node.priority);
                total_bytes_written += writeWithRate(dequeued_node.data, sizeof(dequeued_node.data), write_rate, total_bytes_written, start_time, outfile);
                expected_sequence += 1;
            }

    printf("%d END\n", packet_queue->size);

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

//     PriorityQueue* pq = createPriorityQueue();


//     header_t t1 = create_header(1,0,0,0);
//     header_t t2 = create_header(2,0,0,0);
//     header_t t3 = create_header(3,0,0,0);
//     header_t t4 = create_header(4,0,0,0);

//     packet_t p1 = create_packet(NULL, t1);
//     packet_t p2 = create_packet(NULL, t2);
//     packet_t p3 = create_packet(NULL, t3);
//     packet_t p4 = create_packet(NULL, t4);

//     enqueue(pq, p4.header.seq_num, NULL, 0);
//     enqueue(pq, p2.header.seq_num, NULL, 0);
//     enqueue(pq, p1.header.seq_num, NULL, 0);
//     enqueue(pq, p3.header.seq_num, NULL, 0);

//     while(true){
//     int peak_val = peak(pq);
//     printf("PEAK: %d", peak_val);
//     if (peak_val != -1){
//         dequeue(pq);
//     } else {
//         break;
//     }

// }
}
