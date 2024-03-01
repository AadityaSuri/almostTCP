/**
 * @file packet.h
 * @brief Definitions and functions related to packet headers and creation.
 * @author Connor Johst - cjohst & Aaditya Suri - AadityaSuri
 * @bug No known bugs
 */

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define PAYLOAD_SZ 500 // bytes of data contained within each packet.

// Define flag values for packet headers
#define ACK_FLAG 0b0100000000000000 //Ack flag
#define FIN_FLAG 0b0000010000000000 // Finish flag 


// Macros to check flag values
#define IS_ACK(flags) (flags & ACK_FLAG) //
#define IS_FIN(flags) (flags & FIN_FLAG) //

/**
 * @struct header
 * @brief Structure representing a packet header.
 */

typedef struct header {
    uint32_t seq_num; /**< Sequence number */
    uint32_t ack_num; /**< Acknowledgment number */
    uint16_t length;  /**< Length of the packet */
    uint16_t flags;   /**< Flags associated with the packet */
} header_t;

/**
 * @struct packet
 * @brief Structure representing a packet with header and data.
 */

typedef struct packet {
    header_t header;
    unsigned char data[PAYLOAD_SZ];
} packet_t;

/**
 * @brief Creates a packet header with the specified parameters.
 * 
 * @param seq_number The sequence number of the packet.
 * @param ack_number The acknowledgment number of the packet.
 * @param length The length of the packet.
 * @param flags The flags associated with the packet.
 * @return The created packet header.
 */
header_t create_header(uint32_t seq_number, uint32_t ack_number, uint16_t length, uint16_t flags);

/**
 * @brief Creates a packet with the specified data and header.
 * 
 * @param data The data to be included in the packet.
 * @param pkt_header The header of the packet.
 * @return The created packet.
 */
packet_t create_packet(unsigned char [], header_t pkt_header);

#endif