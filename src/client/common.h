#ifndef MIMUW_SIK_TCP_SOCKETS_COMMON_H
#define MIMUW_SIK_TCP_SOCKETS_COMMON_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdint>
#include <arpa/inet.h>

#include "err.h"

inline static uint16_t read_port(char *string) {
    errno = 0;
    unsigned long port = strtoul(string, nullptr, 10);
    PRINT_ERRNO();
    if (port > UINT16_MAX || port <= 0) {
        exit(1);
    }

    return (uint16_t) port;
}

inline static struct sockaddr_in get_address(char *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, NULL, &hints, &address_result));

//    struct sockaddr_in6 address;
//    address.sin6_family = AF_INET6; // IPv4
//    address.sin6_addr = in6addr_any;
//    .s_addr =
//            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
//    address.sin6_port = htons(port);
    struct sockaddr_in address;
    address.sin_family = AF_INET; // IPv4
    address.sin_addr =
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr; // IP address
    address.sin_port = htons(port);

    freeaddrinfo(address_result);

    return address;
}

//inline static int open_tcp_socket() {
//    int socket_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
//    if (socket_fd < 0) {
//        PRINT_ERRNO();
//    }
//
//    return socket_fd;
//}


inline static int open_udp_socket() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    return socket_fd;
}

//inline static void bind_udp_socket(int socket_fd, uint16_t port) {
//    struct sockaddr_in server_address;
//    server_address.sin_family = AF_INET; // IPv4
//    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
//    server_address.sin_port = htons(port);
//
//    // bind the socket to a concrete address
//    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &server_address,
//                     (socklen_t) sizeof(server_address)));
//}



inline static void start_listening(int socket_fd, size_t queue_length) {
    CHECK_ERRNO(listen(socket_fd, queue_length));
}

inline static uint16_t get_port(struct sockaddr_in *address) {
    return ntohs(address->sin_port);
}

/// No need to free the returned string,
/// it is a pointer to a static buf
inline static char *get_ip(struct sockaddr_in *address) {
    return inet_ntoa(address->sin_addr);
}

inline static int accept_connection(int socket_fd, struct sockaddr_in *client_address) {
    socklen_t
            client_address_length = (socklen_t)
            sizeof(*client_address);

    int client_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_address_length);
    if (client_fd < 0) {
        PRINT_ERRNO();
    }

    return client_fd;
}

inline static void connect_socket(int socket_fd, const struct sockaddr_in *address) {
    CHECK_ERRNO(connect(socket_fd, (struct sockaddr *) address, sizeof(*address)));
}

//inline static void send_message(int socket_fd, const void *message, size_t length, int flags) {
//    errno = 0;
//    ssize_t sent_length = send(socket_fd, message, length, flags);
//    if (sent_length < 0) {
//        PRINT_ERRNO();
//    }
//    ENSURE(sent_length == (ssize_t) length);
//}
inline static struct sockaddr_in get_send_address(const char *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_INET6; // IPv4
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, nullptr, &hints, &address_result));

//    struct sockaddr_in6 send_address;
//    send_address.sin6_family = AF_INET6; // IPv4
//    send_address.sin6_addr = in6addr_any;
//    send_address.sin6_addr.s_addr =
//            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
//    send_address.sin6_port = htons(port); // port from the command line
    struct sockaddr_in address;
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr =
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
    address.sin_port = htons(port);

    freeaddrinfo(address_result);

    return address;
}

inline static void
send_message(int socket_fd, const struct sockaddr_in *client_address, const char *message, size_t length) {
    socklen_t
            address_length = (socklen_t)
            sizeof(*client_address);
    int flags = 0;
    ssize_t sent_length = sendto(socket_fd, message, length, flags,
                                 (struct sockaddr *) client_address, address_length);
    ENSURE(sent_length == (ssize_t) length);
}

inline static size_t receive_message_tcp(int socket_fd, void *buffer, size_t max_length, int flags) {
    errno = 0;
    ssize_t received_length = recv(socket_fd, buffer, max_length, flags);
    if (received_length < 0) {
        PRINT_ERRNO();
    }
    return (size_t) received_length;
}

//inline static size_t receive_message(int socket_fd, struct sockaddr_in *receive_address,
//                       char *buffer, size_t max_length) {
//    socklen_t address_length = (socklen_t)
//    sizeof(*receive_address);
//    errno = 0;
//    ssize_t received_length = recvfrom(socket_fd, buffer, max_length,
//                                       0,
//                                       (struct sockaddr *) receive_address,
//                                       &address_length);
//    if (received_length < 0) {
//        PRINT_ERRNO();
//    }
//    return (size_t) received_length;
//}


#endif //MIMUW_SIK_TCP_SOCKETS_COMMON_H
