#pragma once

#include <stdint.h>

#define URG_FLAG 0b1000000000000000
#define ACK_FLAG 0b0100000000000000
#define PSH_FLAG 0b0010000000000000
#define RST_FLAG 0b0001000000000000
#define SYN_FLAG 0b0000100000000000
#define FIN_FLAG 0b0000010000000000
#define SYN_ACK_FLAG SYN_FLAG | ACK_FLAG

#define IS_ACK(flags) flags & ACK_FLAG
#define IS_SYN(flags) flags & SYN_FLAG
#define IS_FIN(flags) flags & FIN_FLAG
#define IS_SYN_ACK(flags) flags & SYN_ACK_FLAG
#define HAS_FLAGS(flags) IS_ACK(flags) | IS_SYN(flags) | IS_FIN(flags)


typedef struct header {
    uint32_t seq_num;
    uint32_t ack_num;
    uint16_t length;
    uint16_t flags;
    
} header_t;

typedef struct packet {
    header_t header;
    unsigned char data[64];
} packet_t;

header_t create_header(uint32_t seq_number, uint32_t ack_number, uint16_t length, uint16_t flags);
packet_t create_packet(unsigned char [], header_t pkt_header);
