#include "events.h"

using namespace std;

void Event::deserialize_position(int socket_fd, Buffer &buf, struct sockaddr_in6 addr) {
    char buffer[sizeof(uint16_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint16_t), addr); // Position.x
    buf.write_into_buffer(be16toh(*(uint16_t *) buffer));
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint16_t), addr); // Position.y
    buf.write_into_buffer(be16toh(*(uint16_t *) buffer));
}


void Event::deserialize_event(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr) {
    char buffer[1];
    get_n_bytes_from_server(socket_fd, buffer, 1, addr);
    auto event_id = (uint8_t) buffer[0];
    buf.reset_buffer();
    switch (event_id) {
        case BOMB_PLACED:
            INFO("Received BOMB_PLACED event");
            deserialize_bomb_placed(socket_fd, buf, game, addr);
            break;
        case BOMB_EXPLODED:
            INFO("Received BOMB_EXPLODED event");
            deserialize_bomb_exploded(socket_fd, buf, game, addr);
            break;
        case PLAYER_MOVED:
            INFO("Received PLAYER_MOVED event");
            deserialize_player_moved(socket_fd, buf, game, addr);
            break;
        case BLOCK_PLACED:
            INFO("Received BLOCK_PLACED event");
            deserialize_block_placed(socket_fd, buf, game, addr);
            break;
        default:
            fatal("Received unknown event");
            break;
    }
}

void Event::deserialize_bomb_placed(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr) {
    char local_buf[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(bomb_id_t), addr);
    buf.write_into_buffer(be32toh(*(bomb_id_t *) local_buf));
    deserialize_position(socket_fd, buf, addr);
    game.place_bomb(buf);
}

void Event::deserialize_bomb_exploded(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr) {
    char local_buf[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(bomb_id_t), addr);
    buf.write_into_buffer(be32toh(*(bomb_id_t *) local_buf));
    game.explode_bomb(buf);
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(uint32_t), addr);  // list of destroyed robots
    list_len_t list_size = be32toh(*(list_len_t *) local_buf);
    buf.reset_buffer();
    for (list_len_t i = 0; i < list_size; i++) {
        get_n_bytes_from_server(socket_fd, local_buf, sizeof(player_id_t), addr);
        buf.write_into_buffer(*(player_id_t *) local_buf);
        game.kill_player(buf);
    }
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(uint32_t), addr);  // list of destroyed blocks
    list_size = be32toh(*(uint32_t *) local_buf);
    buf.reset_buffer();
    for (uint32_t i = 0; i < list_size; i++) {
        deserialize_position(socket_fd, buf, addr);
        game.destroy_block(buf);
    }
}

void Event::deserialize_player_moved(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr) {
    char buffer[sizeof(player_id_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(player_id_t), addr);
    buf.write_into_buffer(*(player_id_t *) buffer);
    deserialize_position(socket_fd, buf, addr);
    INFO("Player moved to " << buf.get_buffer());
    Buffer::print_buffer(buf.get_buffer(), buf.get_no_written_bytes());
    game.move_player(buf);
}

void Event::deserialize_block_placed(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr) {
    deserialize_position(socket_fd, buf, addr);
    game.place_block(buf);
}
