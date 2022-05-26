#include "player.h"

void read_player(int socket_fd, Buffer &buf) {
    char buffer[1];
    get_n_bytes_from_server(socket_fd, buffer, 1);
    buf.write_into_buffer((uint8_t) buffer[0]);                  // player id
    read_str(socket_fd, buf);                                  // player name
    read_str(socket_fd, buf);                                  // player address
}


void Player::generate_respond(Buffer &buf){
    buf.write_into_buffer((uint8_t) name.size());
    buf.write_into_buffer(name.c_str(), (size_t) name.size());
    buf.write_into_buffer((uint8_t) addr.size());
    buf.write_into_buffer(addr.c_str(), (size_t) addr.size());
}
