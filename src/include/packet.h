#include <stdint.h>


typedef struct header {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t sequence;
    uint32_t ack;
    uint32_t flags;
    uint32_t checksum;
} header_t;

typedef struct packet {
    header_t header;
    unsigned char data [64];
} packet_t;

packet_t* create_packet(uint16_t source_port, uint16_t dest_port, uint32_t seq_number, uint32_t ack_number, uint32_t flags);

uint32_t compute_checksum(packet_t* packet);
