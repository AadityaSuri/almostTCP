struct header {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t sequence;
    uint32_t ack;
    uint32_t flags;
    uint32_t checksum;
};

struct packet {
    struct header header;
    unsigned char* data;
};

