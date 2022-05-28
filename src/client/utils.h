#ifndef UTILS_H
#define UTILS_H

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>

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

size_t get_n_bytes_from_server(int socket_fd, void *buffer, size_t n, struct sockaddr_in6 addr);

int bind_udp_socket(port_t port);

int bind_tcp_socket(port_t port);

void deserialize_str(int socket_fd, Buffer &buf, struct sockaddr_in6 addr);

//inline static int open_tcp_ip6_socket(struct sockaddr_in6 addr) {
//    int socket_fd = socket(addr., SOCK_STREAM, IPPROTO_TCP);
//    if (socket_fd < 0) {
//        PRINT_ERRNO();
//    }
//    INFO("TCP socket created");
//    return socket_fd;
//}

inline static int open_udp_ip6_socket() {
    int socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    return socket_fd;
}

inline static void bind_ip6_socket(int socket_fd, uint16_t port) {
    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_flowinfo = 0;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);
    address.sin6_scope_id = 0;
    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &address,
                     (socklen_t) sizeof(address)));
    INFO("Socket bound to port " << port);
}

inline static void connect_socket(int socket_fd, const struct sockaddr_in6 *address) {
    INFO("Connecting to " << inet_ntop(AF_INET6, &address->sin6_addr,
                                       (char *) &address->sin6_addr,
                                       sizeof(address->sin6_addr)) << ":"
                          << ntohs(address->sin6_port));
    CHECK_ERRNO(connect(socket_fd, (struct sockaddr *) address, sizeof(*address)));
}

inline static struct sockaddr_in6 get_tcp_address(char *host, uint16_t port) {
    INFO("Getting address for host " << host << " and port " << port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, nullptr, &hints, &address_result));

    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_addr = ((struct sockaddr_in6 *) (address_result->ai_addr))->sin6_addr; // IP address
    address.sin6_port = htons(port);

    freeaddrinfo(address_result);

    INFO("TCP address: " << inet_ntop(AF_INET6, &address.sin6_addr, host, INET6_ADDRSTRLEN));
    return address;
}

inline static struct sockaddr_in6 get_udp_address(char *host, uint16_t port) {
    INFO("Getting address for host " << host << " and port " << port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
//    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, nullptr, &hints, &address_result));

    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_addr = ((struct sockaddr_in6 *) (address_result->ai_addr))->sin6_addr; // IP address
    address.sin6_port = htons(port);

    freeaddrinfo(address_result);

    INFO("UDP address: " << inet_ntop(AF_INET6, &address.sin6_addr, host, INET6_ADDRSTRLEN));
    return address;
}

#endif //UTILS_H
