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

    Position(uint16_t x, uint16_t y) : x(x), y(y) {};
};

struct Bomb {
    Position pos;
    uint16_t timer;

    Bomb(Position pos, uint16_t timer) : pos(pos), timer(timer) {};

    void generate_respond(Buffer &buf) const {
        buf.write_into_buffer(pos.x);
        buf.write_into_buffer(pos.y);
        buf.write_into_buffer(timer);
    }
};

size_t get_n_bytes_from_server(int socket_fd, void *buffer, const size_t n);

//void read_position(Buffer &buf){
//    char buffer[sizeof(uint16_t)];
//    get_n_bytes_from_server(buffer, sizeof(uint16_t)); // Position.x
//    buf.write_into_buffer(*(uint16_t *) buffer);
//    get_n_bytes_from_server(buffer, sizeof(uint16_t)); // Position.y
//    buf.write_into_buffer(*(uint16_t *) buffer);
//}


#endif //UTILS_H
