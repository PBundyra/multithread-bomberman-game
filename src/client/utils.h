#ifndef UTILS_H
#define UTILS_H

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>

#include "buffer.h"
#include "err.h"

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

#define INFO(x)                                     \
    do {                                            \
        if (debug) {                                \
            std::cerr << x << std::endl;            \
        }                                           \
    } while (0)

using str_len_t = uint8_t;
using list_len_t = uint32_t;
using map_len_t = uint32_t;
using port_t = uint16_t;
using Position = std::pair<uint16_t, uint16_t>;

size_t get_n_bytes_from_server(int socket_fd, void *buffer, size_t n);

int bind_socket(port_t port);

void read_str(int socket_fd, Buffer &buf);


#endif //UTILS_H
