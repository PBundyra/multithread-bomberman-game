#ifndef UTILS_H
#define UTILS_H

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <iostream>

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

//using uint8_t = uint8_t;
//using uint32_t = uint32_t;
//using uint32_t = uint32_t;
//using uint16_t = uint16_t;
//using uint16_t = uint16_t;
using Position = std::pair<uint16_t, uint16_t>;


namespace udp {
    inline static struct addrinfo *get_addr_info(char *host, char *port) {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_socktype = SOCK_DGRAM;

        struct addrinfo *address_result;
        Err::check_errno(getaddrinfo(host, port, &hints, &address_result));
        return address_result;
    }

    inline static int connect_with_gui(struct addrinfo *server_addr_info) {
        int socket_fd = socket(server_addr_info->ai_family, server_addr_info->ai_socktype, 0);
        Err::ensure(socket_fd >= 0);
        Err::check_errno(connect(socket_fd, server_addr_info->ai_addr, server_addr_info->ai_addrlen));
        return socket_fd;
    }

    inline static int bind_udp_socket(uint16_t port) {
        int socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
        Err::ensure(socket_fd >= 0);

        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        addr.sin6_addr = in6addr_any;

        Err::check_errno(bind(socket_fd, (struct sockaddr *) &addr, (socklen_t) sizeof(addr)));
        return socket_fd;
    }
}

namespace tcp {
    inline static struct addrinfo *get_addr_info(char *host, char *port) {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_socktype = SOCK_STREAM;

        struct addrinfo *address_result;
        Err::check_errno(getaddrinfo(host, port, &hints, &address_result));
        return address_result;
    }

    inline static int connect_with_server(struct addrinfo *server_addr_info) {
        int socket_fd = socket(server_addr_info->ai_family, server_addr_info->ai_socktype, 0);
        Err::ensure(socket_fd >= 0);
        Err::check_errno(connect(socket_fd, server_addr_info->ai_addr, server_addr_info->ai_addrlen));
        int yes = 1;
        Err::check_errno(setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int)));
        return socket_fd;
    }
}

inline static size_t get_n_bytes_from_server(int socket_fd, void *buffer, const size_t n) {
    errno = 0;
    ssize_t received_length = recv(socket_fd, buffer, n, MSG_WAITALL);
    if (received_length < 0) {
        Err::print_errno();
    } else if (received_length == 0) {
        INFO("Connection closed by server");
        exit(EXIT_SUCCESS);
    }
    INFO("Received " << received_length << " bytes from server");
    return (size_t) received_length;
}

inline static uint8_t get_uint8_t_from_server(int socket_fd) {
    char local_buf[sizeof(uint8_t)];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(uint8_t));
    return *(uint8_t *) local_buf;
}

inline static uint16_t get_uint16_t_from_server(int socket_fd) {
    char local_buf[sizeof(uint16_t)];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(uint16_t));
    return be16toh(*(uint16_t *) local_buf);
}

inline static uint32_t get_uint32_t_from_server(int socket_fd) {
    char local_buf[sizeof(uint32_t)];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(uint32_t));
    return be32toh(*(uint32_t *) local_buf);
}

inline static void read_str(int socket_fd, Buffer &buf) {
    char local_buf[MAX_STR_LEN];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(uint8_t));
    auto str_len = (uint8_t) local_buf[0];
    buf.write_into_buffer(str_len);
    get_n_bytes_from_server(socket_fd, local_buf, str_len);
    buf.write_into_buffer(local_buf, str_len);
}

#endif //UTILS_H
