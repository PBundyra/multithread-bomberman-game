//
// Created by patryk on 17.05.22.
//

#include "utils.h"

size_t get_n_bytes_from_server(int socket_fd, void *buffer, const size_t n){
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