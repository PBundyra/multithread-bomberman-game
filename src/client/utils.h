#ifndef UTILS_H
#define UTILS_H

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <netdb.h>

#include "buffer.h"
#include "err.h"

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

#define MAX_STR_LEN 256

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
using cord_t = uint16_t;
using Position = std::pair<cord_t, cord_t>;

size_t get_n_bytes_from_server(int socket_fd, void *buffer, size_t n);

int bind_udp_socket(port_t port);

int bind_tcp_socket(port_t port);

void read_str(int socket_fd, Buffer &buf);

inline static int open_tcp_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    return socket_fd;
}

inline static int open_udp_ip6_socket() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    return socket_fd;
}

inline static void bind_ip6_socket(int socket_fd, uint16_t port) {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);


//    struct sockaddr_in6 address;
//    address.sin6_family = AF_INET6;
//    address.sin6_flowinfo = 0;
//    address.sin6_addr = in6addr_any;
//    address.sin6_port = htons(port);
//    address.sin6_scope_id = 0;
    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &address,
                     (socklen_t) sizeof(address)));
}

inline static struct sockaddr_in6 get_send_address(const char *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_INET6; // IPv4
//    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, nullptr, &hints, &address_result));

    struct sockaddr_in6 send_address;
    send_address.sin6_family = AF_INET6; // IPv4
    send_address.sin6_addr = in6addr_any;
    send_address.sin6_addr =
            ((struct sockaddr_in6 *) (address_result->ai_addr))->sin6_addr; // IP address
    send_address.sin6_port = htons(port); // port from the command line
//    struct sockaddr_in address;
//    address.sin_family = AF_INET; // IPv4
//    address.sin_addr.s_addr =
//            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
//    address.sin_port = htons(port);

    freeaddrinfo(address_result);

    return send_address;
}


#endif //UTILS_H
