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

struct packet create_packet(uint16_t source_port, uint16_t dest_port, uint32_t seq_number, uint32_t ack_number, uint32_t flags){

    struct packet created_packet;

    created_packet->source_port = source_port;
    created_packet->dest_port = dest_port;
    created_packet->sequence = seq_number;
    created_packet->ack = ack_number;
    created_packet->flags = flags;
    // created_packet->checksum = compute checksum
    return created_packet;

}