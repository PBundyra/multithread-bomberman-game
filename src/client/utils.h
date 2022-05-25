#ifndef UTILS_H
#define UTILS_H

#include "buffer.h"

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Position {
    uint16_t x;
    uint16_t y;
};

//void read_position(Buffer &buf){
//    char buffer[sizeof(uint16_t)];
//    get_n_bytes_from_server(buffer, sizeof(uint16_t)); // Position.x
//    buf.write_into_buffer(*(uint16_t *) buffer);
//    get_n_bytes_from_server(buffer, sizeof(uint16_t)); // Position.y
//    buf.write_into_buffer(*(uint16_t *) buffer);
//}


#endif //UTILS_H
