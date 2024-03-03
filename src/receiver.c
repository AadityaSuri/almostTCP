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

size_t writeWithRate(char data[], int data_len, unsigned long long int write_rate, size_t total_bytes_written, time_t start_time, FILE *outfile)
{
    size_t bytes_written = 0;
    time_t current_time;
    time(&current_time);

    if (write_rate == 0)
    {
        bytes_written = fwrite(data, sizeof(char), data_len, outfile);
        fflush(outfile);
        return bytes_written;
    }

    //logic to handle rate limited writing. We compute how writing the whole file would affect the write rate and sleep until the affect would not increase the rate over the maximum
    double elapsed_seconds = difftime(current_time, start_time);
    double write_rate_if_all_written = ((double)total_bytes_written + (double)data_len) / elapsed_seconds;

    while (write_rate_if_all_written > (double)write_rate)
    {
        sleep(0.25);
        time(&current_time);
        elapsed_seconds = difftime(current_time, start_time);
        write_rate_if_all_written = ((double)total_bytes_written + (double)data_len) / elapsed_seconds;
    }

    for (size_t i = 0; i < data_len; i++)
    {
        bytes_written += fprintf(outfile, "%c", data[i]);
    }
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
void rrecv(unsigned short int udp_port,
           char *destination_file,
           unsigned long long int write_rate)
{

    time_t start_time;
    time(&start_time);

    FILE *outfile = fopen(destination_file, "w");

    packet_t incoming_packet, outgoing_packet;
    header_t outgoing_header;
    PriorityQueue *packet_queue = createPriorityQueue();

    int recv_len, send_len;
    struct sockaddr_in server_addr, client_addr;

    int sock_fd = socket(
        AF_INET,
        SOCK_DGRAM,
        0);

    if (sock_fd < 0)
    {
        fprintf(stderr, "Socket creation failed: %d\n", sock_fd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = udp_port;

    int bind_code = bind(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_code < 0)
    {
        fprintf(stderr, "Socket bind failed: %d\n", bind_code);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    bool connection_open = true;

    uint32_t expected_sequence = 0;
    uint32_t ack_number;
    size_t total_bytes_written = 0;

    int len = sizeof(client_addr);

    struct timeval tic, toc;
    bool first_packet_received = false;


    printf("Started listening on port %d\n", udp_port);

    memset(&incoming_packet, 0, sizeof(incoming_packet));
    while (!(IS_FIN(incoming_packet.header.flags && IS_ACK(incoming_packet.header.flags))))
    {

        recv_len = recvfrom(sock_fd, &incoming_packet, sizeof(incoming_packet), 0, (const struct sock_addr *)&client_addr, &len);
        if (recv_len == -1)
        {
            if (errno == EAGAIN) {
                printf("TIMEOUT\n");
                break;
            }
            fprintf(stderr, "Receive failed: %d\n", recv_len);
            close(sock_fd);
        }
        ack_number = incoming_packet.header.seq_num;

        if (!first_packet_received) {
            gettimeofday(&tic, NULL);
            first_packet_received = true;
            struct timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        }

        if (IS_FIN(incoming_packet.header.flags))
        {   
            // send fin ack
            printf("Received FIN\n");

            packet_t fin_ack_packet;
            fin_ack_packet = create_packet(NULL, create_header(0, 0, -1, FIN_FLAG | ACK_FLAG));
            sendto(sock_fd, &fin_ack_packet, sizeof(packet_t), 0, (const struct sockaddr *)&client_addr, len);
            // connection_open = false;
            
            gettimeofday(&toc, NULL);
            double elapsed_time = (double) (toc.tv_sec - tic.tv_sec) * 1000.0 + 
                                     (double) (toc.tv_usec - tic.tv_usec) / 1000.0;
            printf("Elapsed time: %.3f ms\n", elapsed_time);
            break;

        }
        else
        {
            if (incoming_packet.header.seq_num < expected_sequence)
            {
                //remove data field so if next packet received does not have a full data field no data remains. 
                memset(&incoming_packet.data, 0, incoming_packet.header.length);
            }
            else if (incoming_packet.header.seq_num > expected_sequence)
            {
                // enqueue packet with priority seq_num to be written later
                enqueue(packet_queue, incoming_packet.header.seq_num, incoming_packet.data, incoming_packet.header.length);
            }
            else
            {
                // write packet
                total_bytes_written += writeWithRate(incoming_packet.data, incoming_packet.header.length, write_rate, total_bytes_written, start_time, outfile);
                expected_sequence += 1;
            }
            // check if ANY enqueued data can be written and write it
            while (peak(packet_queue) <= expected_sequence && peak(packet_queue) != -1)
            {
                if (peak(packet_queue) < expected_sequence)
                {
                    // ensure we never write the same packet twice
                    QueueNode dequeued_node = dequeue(packet_queue);
                    continue;
                }
                QueueNode dequeued_node = dequeue(packet_queue);
                total_bytes_written += writeWithRate(dequeued_node.data, dequeued_node.data_len, write_rate, total_bytes_written, start_time, outfile);
                expected_sequence += 1;
            }
            // send an ack with ack_number = sequence_number received
            outgoing_header = create_header(0, ack_number, incoming_packet.header.length, ACK_FLAG);
            outgoing_packet = create_packet(NULL, outgoing_header);
            send_len = sendto(sock_fd, &outgoing_packet, sizeof(outgoing_packet), 0, (const struct sock_addr *)&client_addr, len);
            if (send_len < 0)
            {
                fprintf(stderr, "Ack send failed: %d\n", send_len);
                exit(EXIT_FAILURE);
            }
        }
    }
    printf("connection closed\n");

    fclose(outfile);

    close(sock_fd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

    unsigned short int udp_port;
    char *destination_file;
    unsigned long long int write_rate;

    if (argc != 4)
    {
        fprintf(stderr, "usage: %s UDP_port filename_to_write writerate\n\n", argv[0]);
        exit(1);
    }

    udp_port = (unsigned short int)atoi(argv[1]);
    destination_file = argv[2];
    write_rate = (unsigned long long int)atoi(argv[3]);

    rrecv(udp_port, destination_file, write_rate);
}
