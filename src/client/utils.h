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

int bind_udp_socket(port_t port);

int bind_tcp_socket(port_t port);

void read_str(int socket_fd, Buffer &buf);

inline static int open_tcp_socket() {
    int socket_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    return socket_fd;
}

inline static int open_udp_ip6_socket() {
    int socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    return socket_fd;
}

inline static void bind_ip6_socket(int socket_fd, uint16_t port) {
    struct sockaddr_in6 address;
    address.sin6_family = AF_INET6;
    address.sin6_flowinfo = 0;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);
    address.sin6_scope_id = 0;
    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &address,
                     (socklen_t) sizeof(address)));
}


#endif //UTILS_H
