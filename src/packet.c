#include <stdint.h>
#include <stdlib.h>

#include "packet.h"



header_t create_header(uint32_t seq_number, uint32_t ack_number, uint16_t length, uint16_t flags){

    header_t created_header;
    
    created_header.seq_num = seq_number;
    created_header.ack_num = ack_number;
    created_header.flags = flags;
    created_header.length = length;
    return created_header;

}

packet_t create_packet(unsigned char data[], header_t pkt_header) {
  packet_t created_packet; 
  created_packet.header = pkt_header;


  if (data){
    for (size_t i = 0; i < sizeof(created_packet.data); i++) {
      created_packet.data[i] = data[i];
    }
  }

  return created_packet;
}
