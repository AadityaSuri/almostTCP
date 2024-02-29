/**
 * @file packet.c
 * @brief Functions for creating packets and packet headers.
 * @author Connor Johst - cjohst & Aaditya Suri - AadityaSuri
 * @bug No known bugs
 */

#include <stdint.h>
#include <stdlib.h>

#include "packet.h"

/**
 * @brief Creates a packet header with the specified parameters.
 * 
 * @param seq_number The sequence number of the packet.
 * @param ack_number The acknowledgment number of the packet.
 * @param length The length of the packet.
 * @param flags The flags associated with the packet.
 * @return The created packet header.
 */

header_t create_header(uint32_t seq_number, uint32_t ack_number, uint16_t length, uint16_t flags){

    header_t created_header;
    
    created_header.seq_num = seq_number;
    created_header.ack_num = ack_number;
    created_header.flags = flags;
    created_header.length = length;
    return created_header;

}

/**
 * @brief Creates a packet with the specified data and header.
 * 
 * @param data The data to be included in the packet.
 * @param pkt_header The header of the packet.
 * @return The created packet.
 */

packet_t create_packet(unsigned char data[], header_t pkt_header) {
  packet_t created_packet; 
  created_packet.header = pkt_header;


  if (data){
    for (size_t i = 0; i < created_packet.header.length; i++) {
      created_packet.data[i] = data[i];
    }
  }

  return created_packet;
}
