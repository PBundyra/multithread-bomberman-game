#ifndef UTILS_H
#define UTILS_H

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "buffer.h"
#include "err.h"

using list_len_t = uint32_t;
using map_len_t = uint32_t;
using port_t = uint16_t;

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

using Position = std::pair<uint16_t, uint16_t>;

//struct Position {
//    uint16_t x;
//    uint16_t y;
//
//    Position(uint16_t x, uint16_t y) : x(x), y(y) {};
//};


size_t get_n_bytes_from_server(int socket_fd, void *buffer, size_t n);

int bind_socket(port_t port);

void read_str(int socket_fd, Buffer &buf);


#endif //UTILS_H
