#ifndef PACKET_H
#define PACKET_H

/**
 * @file packet.h
 * @brief Definitions and functions related to packet headers and creation.
 */

#include <stdint.h>

#define PAYLOAD_SZ 500

// Define flag values for packet headers
#define URG_FLAG 0b1000000000000000 /**< Urgent flag */
#define ACK_FLAG 0b0100000000000000 /**< Acknowledgment flag */
#define PSH_FLAG 0b0010000000000000 /**< Push flag */
#define RST_FLAG 0b0001000000000000 /**< Reset flag */
#define SYN_FLAG 0b0000100000000000 /**< Synchronize flag */
#define FIN_FLAG 0b0000010000000000 /**< Finish flag */
#define SYN_ACK_FLAG SYN_FLAG | ACK_FLAG /**< Syn-Ack flag combination */

// Macros to check flag values
#define IS_ACK(flags) (flags & ACK_FLAG) /**< Check if Acknowledgment flag is set */
#define IS_SYN(flags) (flags & SYN_FLAG) /**< Check if Synchronize flag is set */
#define IS_FIN(flags) (flags & FIN_FLAG) /**< Check if Finish flag is set */
#define IS_SYN_ACK(flags) (flags & SYN_ACK_FLAG) /**< Check if Syn-Ack flag combination is set */
#define HAS_FLAGS(flags) (IS_ACK(flags) | IS_SYN(flags) | IS_FIN(flags)) /**< Check if any flags are set */

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