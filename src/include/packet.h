#include <stdint.h>


typedef struct header {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t sequence;
    uint32_t ack;
    uint32_t flags;
    // uint32_t checksum;
} header_t;

typedef struct packet {
    header_t* header;
    unsigned char data[64];
} packet_t;

header_t* create_header(uint16_t source_port, uint16_t dest_port, uint32_t seq_number, uint32_t ack_number, uint32_t flags);
packet_t* create_packet(unsigned char [], header_t* pkt_header);

void destroy_packet(packet_t* packet);

// uint32_t compute_checksum(packet_t* packet);
