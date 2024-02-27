#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "packet.h"

typedef struct pkt_list {
    size_t length;
    size_t item_size;
    packet_t** items;
} pkt_list_t;

pkt_list_t* create_list(size_t item_size);


