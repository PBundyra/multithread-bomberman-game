#include "events.h"

using namespace std;

void read_position(int socket_fd, Buffer &buf) {
    char buffer[sizeof(uint16_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint16_t)); // Position.x
    buf.write_into_buffer(*(uint16_t *) buffer);
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint16_t)); // Position.y
    buf.write_into_buffer(*(uint16_t *) buffer);
}


void read_event(int socket_fd, Buffer &buf, Game &game) {
    char buffer[1];
    get_n_bytes_from_server(socket_fd, buffer, 1);
    auto event_id = (uint8_t) buffer[0];
    buf.reset_buffer();
    switch (event_id) {
        case BOMB_PLACED:
            INFO("Received BOMB_PLACED event");
            read_bomb_placed(socket_fd, buf, game);
            break;
        case BOMB_EXPLODED:
            INFO("Received BOMB_EXPLODED event");
            read_bomb_exploded(socket_fd, buf, game);
            break;
        case PLAYER_MOVED:
            INFO("Received PLAYER_MOVED event");
            read_player_moved(socket_fd, buf, game);
            break;
        case BLOCK_PLACED:
            INFO("Received BLOCK_PLACED event");
            read_block_placed(socket_fd, buf, game);
            break;
        default:
            fatal("Received unknown event");
            break;
    }
}

void read_bomb_placed(int socket_fd, Buffer &buf, Game &game) {
    char local_buf[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(bomb_id_t));
    buf.write_into_buffer(*(bomb_id_t *) local_buf);
    read_position(socket_fd, buf);
    game.place_bomb(buf);
}

void read_bomb_exploded(int socket_fd, Buffer &buf, Game &game) {
    char local_buf[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(bomb_id_t));
    buf.write_into_buffer(*(bomb_id_t *) local_buf);
    game.explode_bomb(buf);
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(list_len_t));  // list of destroyed robots
    list_len_t list_len = be32toh(*(uint32_t *) local_buf);
    buf.reset_buffer();
    for (list_len_t i = 0; i < list_len; i++) {
        get_n_bytes_from_server(socket_fd, local_buf, sizeof(player_id_t));
        buf.write_into_buffer(*(player_id_t *) local_buf);
        game.kill_player(buf);
    }
    buf.reset_buffer();
    get_n_bytes_from_server(socket_fd, local_buf, sizeof(list_len_t));  // list of destroyed blocks
    list_len = be32toh(*(uint32_t *) local_buf);
    buf.reset_buffer();
    for (list_len_t i = 0; i < list_len; i++) {
        read_position(socket_fd, buf);
        game.destroy_block(buf);
    }
}

void read_player_moved(int socket_fd, Buffer &buf, Game &game) {
    char buffer[sizeof(player_id_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(player_id_t));
    buf.write_into_buffer(*(player_id_t *) buffer);
    read_position(socket_fd, buf);
    INFO("Player moved to " << buf.get_buffer());
    Buffer::print_buffer(buf.get_buffer(), buf.get_no_written_bytes());
    game.move_player(buf);
}

void read_block_placed(int socket_fd, Buffer &buf, Game &game) {
    read_position(socket_fd, buf);
    game.place_block(buf);
}
