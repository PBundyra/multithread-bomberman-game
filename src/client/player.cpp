#include "player.h"

void Player::read_player(int socket_fd, Buffer &buf) {
    char buffer[sizeof(player_id_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(player_id_t));
    buf.write_into_buffer((player_id_t) buffer[0]);                 // player id
    read_str(socket_fd, buf);                                         // player name
    read_str(socket_fd, buf);                                         // player address
}

void Player::serialize_player(Buffer &buf) {
    buf.write_into_buffer((uint8_t) name.size());                   // player name size
    buf.write_into_buffer(name.c_str(), (size_t) name.size());  // player name
    buf.write_into_buffer((uint8_t) addr.size());                   // player address size
    buf.write_into_buffer(addr.c_str(), (size_t) addr.size());  // player address
}
