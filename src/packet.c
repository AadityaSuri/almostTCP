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

header_t* create_header(uint16_t source_port, uint16_t dest_port, uint32_t seq_number, uint32_t ack_number, uint32_t flags){

    // packet_t* created_packet = (packet_t*)malloc(sizeof(packet_t));
    header_t* created_header = (header_t*)malloc(sizeof(header_t));
    // created_packet->header = created_header;
    
    created_header->source_port = source_port;
    created_header->dest_port = dest_port;
    created_header->sequence = seq_number;
    created_header->ack = ack_number;
    created_header->flags = flags;
    // created_header->checksum = compute_checksum(created_packet);
    return created_header;

}

packet_t* create_packet(unsigned char data[], header_t* pkt_header) {
  packet_t* created_packet = (packet_t*)malloc(sizeof(packet_t));
  created_packet->header = pkt_header;

  for (size_t i = 0; i < sizeof(created_packet->data); i++) {
    created_packet->data[i] = data[i];
  }

  return created_packet;
}

void destroy_packet(packet_t* packet) {
  free(packet->header);
  free(packet);
}

// uint32_t compute_checksum(packet_t* packet) {
//   uint32_t checksum = 0; 
//     // (uint32_t)packet->header->source_port + 
//     // (uint32_t)packet->header->dest_port +
//     // packet->header->sequence +
//     // packet->header->ack + 
//     // packet->header->flags;
//   return checksum;
// }
