#include "utils.h"

size_t get_n_bytes_from_server(int socket_fd, void *buffer, const size_t n) {
    errno = 0;
    ssize_t received_length = recv(socket_fd, buffer, n, MSG_WAITALL);
    if (received_length < 0) {
        PRINT_ERRNO();
    } else if (received_length == 0) {
        std::cout << "Server closed connection" << std::endl;
        exit(0);
    }
    std::cout << "Received message from server of length: " << received_length << "\n";
    return (size_t) received_length;
}

int bind_socket(port_t port) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    ENSURE(socket_fd > 0);

    struct sockaddr_in server_address{};
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port);

    // bind the socket to a concrete address
    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &server_address,
                     (socklen_t) sizeof(server_address)));

    return socket_fd;
}

void read_str(int socket_fd, Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, buffer, 1);
    auto str_len = (uint8_t) buffer[0];
    buf.write_into_buffer(str_len);
    get_n_bytes_from_server(socket_fd, buffer, str_len);
    buf.write_into_buffer(buffer, str_len);
}
