#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "packet.h"

typedef struct d_list {
    size_t length;
    size_t item_size;
    void** items;
} d_list_t;

d_list_t* create_list(size_t item_size);
void append_packet(d_list_t* list, void* item);


