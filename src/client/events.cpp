#include "events.h"

using namespace std;

void read_position(int socket_fd, Buffer &buf) {
    char buffer[sizeof(uint16_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint16_t)); // Position.x
    buf.write_into_buffer(be16toh(*(uint16_t *) buffer));
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint16_t)); // Position.y
    buf.write_into_buffer(be16toh(*(uint16_t *) buffer));
}


void read_event(int socket_fd, Buffer &buf, Game &game) {
    char buffer[1];
    get_n_bytes_from_server(socket_fd, buffer, 1);
    auto event_id = (uint8_t) buffer[0];
    buf.reset_buffer();
    switch (event_id) {
        case BOMB_PLACED:
            cout << "Received BOMB_PLACED" << endl;
            read_bomb_placed(socket_fd, buf, game);
            break;
        case BOMB_EXPLODED:
            cout << "Received BOMB_EXPLODED" << endl;
            read_bomb_exploded(socket_fd, buf, game);
            break;
        case PLAYER_MOVED:
            cout << "Received PLAYER_MOVED" << endl;
            read_player_moved(socket_fd, buf, game);
            break;
        case BLOCK_PLACED:
            cout << "Received BLOCK_PLACED" << endl;
            read_block_placed(socket_fd, buf, game);
            break;
        default:
            cout << "Received unknown event" << endl;
            // TODO handle unknown event
            break;
    }
}


void read_bomb_placed(int socket_fd, Buffer &buf, Game &game) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(bomb_id_t));
    buf.write_into_buffer(*(bomb_id_t *) buffer);
    read_position(socket_fd, buf);
    game.place_bomb(buf);
}

void read_bomb_exploded(int socket_fd, Buffer &buf, Game &game) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(bomb_id_t));
    buf.write_into_buffer(*(bomb_id_t *) buffer);
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint32_t));  // list of destroyed robots
    uint32_t list_size = *(uint32_t *) buffer;
    buf.write_into_buffer(list_size);
    for (uint32_t i = 0; i < list_size; i++) {
        get_n_bytes_from_server(socket_fd, buffer, sizeof(player_id_t));
        buf.write_into_buffer(*(player_id_t *) buffer);
    }
    get_n_bytes_from_server(socket_fd, buffer, sizeof(uint32_t));  // list of destroyed blocks
    list_size = *(uint32_t *) buffer;
    buf.reset_buffer();
    for (uint32_t i = 0; i < list_size; i++) {
        read_position(socket_fd, buf);
        game.destroy_block(buf);
    }
}

void read_player_moved(int socket_fd, Buffer &buf, Game &game) {
    char buffer[sizeof(player_id_t)];
    get_n_bytes_from_server(socket_fd, buffer, sizeof(player_id_t));
    buf.write_into_buffer(*(player_id_t *) buffer);
    read_position(socket_fd, buf);
    game.move_player(buf);
}

void read_block_placed(int socket_fd, Buffer &buf, Game &game) {
    read_position(socket_fd, buf);
    game.place_block(buf);
}
