#include "player.h"

void deserialize_player(int socket_fd, Buffer &buf, struct sockaddr_in6 addr) {
    char buffer[sizeof(player_id_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(player_id_t), addr);
    buf.write_into_buffer((player_id_t) buffer[0]);                 // player id
    deserialize_str(socket_fd, buf, sockaddr_in6());                                         // player name
    deserialize_str(socket_fd, buf, sockaddr_in6());                                         // player address
}

void Player::generate_respond(Buffer &buf){
    buf.write_into_buffer((uint8_t) name.size());                   // player name size
    buf.write_into_buffer(name.c_str(), (size_t) name.size());  // player name
    buf.write_into_buffer((uint8_t) addr.size());                   // player address size
    buf.write_into_buffer(addr.c_str(), (size_t) addr.size());  // player address
}
