#include <stdint.h>
#include <stdlib.h>

#include "packet.h"

#define URG_FLAG 0000000000_100000_0000000000000000
#define ACK_FLAG 0000000000_010000_0000000000000000
#define PSH_FLAG 0000000000_001000_0000000000000000
#define RST_FLAG 0000000000_000100_0000000000000000
#define SYN_FLAG 0000000000_000010_0000000000000000
#define FIN_FLAG 0000000000_000001_0000000000000000
#define SYN_ACK_FLAG SYN_FLAG | ACK_FLAG

#define IS_ACK(flags) flags & ACK_FLAG
#define IS_SYN(flags) flags & SYN_FLAG
#define IS_FIN(flags) flags & FIN_FLAG
#define IS_SYN_ACK(flags) flags & SYN_ACK_FLAG
#define HAS_FLAGS(flags) IS_ACK(flags) | IS_SYN(flags) | IS_FIN(flags)

packet_t* create_packet(uint16_t source_port, uint16_t dest_port, uint32_t seq_number, uint32_t ack_number, uint32_t flags){

    packet_t* created_packet = (packet_t*)malloc(sizeof(packet_t));
    
    created_packet->header.source_port = source_port;
    created_packet->header.dest_port = dest_port;
    created_packet->header.sequence = seq_number;
    created_packet->header.ack = ack_number;
    created_packet->header.flags = flags;
    // created_packet->checksum = compute checksum
    return created_packet;

}
