#include "dynamic_list.h"

d_list_t* create_list(size_t item_size) {
    d_list_t* list = (d_list_t*) calloc(1, sizeof(struct d_list));
    list->item_size = item_size;
    list->length = 0;
    list->items = (void*) 0;
    return list;
}

