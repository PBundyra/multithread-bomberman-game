#include "utils.h"

size_t get_n_bytes_from_server(int socket_fd, void *buffer, const size_t n, struct sockaddr_in6 addr) {
    errno = 0;
    ssize_t received_length = recv(socket_fd, buffer, n, MSG_WAITALL);
    auto address_length = (socklen_t) sizeof(addr);
//    ssize_t received_length = recvfrom(socket_fd, buffer, n, MSG_WAITALL, (struct sockaddr *) &addr,
//                                       &address_length);
    INFO("Received " << received_length << " bytes from server");
    if (received_length < 0) {
        PRINT_ERRNO();
    } else if (received_length == 0) {
        std::cout << "Server closed connection" << std::endl;
        exit(0);
    }
    std::cout << "Received message from server of length: " << received_length << "\n";
    return (size_t) received_length;
}

int bind_udp_socket(port_t port) {
    int socket_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    ENSURE(socket_fd > 0);

    struct sockaddr_in6 server_address{};
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port);

    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &server_address,
                     (socklen_t) sizeof(server_address)));
    return socket_fd;
}

int bind_tcp_socket(port_t port) {
    int socket_fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_TCP);
    ENSURE(socket_fd > 0);

    struct sockaddr_in6 server_address{};
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port);

    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &server_address,
                     (socklen_t) sizeof(server_address)));
    return socket_fd;
}

void deserialize_str(int socket_fd, Buffer &buf, struct sockaddr_in6 addr) {
    char local_buf[256];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(str_len_t), addr);
    auto str_len = (str_len_t) local_buf[0];
    buf.write_into_buffer(str_len);
    get_n_bytes_from_server(socket_fd, local_buf, str_len, addr);
    buf.write_into_buffer(local_buf, str_len);
}
