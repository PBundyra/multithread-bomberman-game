#include "events.h"

void read_event(Buffer &buf) {
    char buffer[1];
    get_n_bytes_from_server(buffer, 1);
    uint8_t event_id = (uint8_t) buffer[0];
    buf.write_into_buffer(event_id);
    switch (event_id) {
        case BOMB_PLACED:
            read_bomb_placed(buf);
            break;
        case BOMB_EXPLODED:
            read_bomb_exploded(buf);
            break;
        case PLAYER_MOVED:
            read_player_moved(buf);
            break;
        case BLOCK_PLACED:
            read_block_placed(buf);
            break;
        default:
            cout << "Received unknown event" << endl;
            // TODO handle unknown event
            break;
    }
}

void read_bomb_placed(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, sizeof(bomb_id_t));
    buf.write_into_buffer(*(bomb_id_t *) buffer);
    read_position(buf);
}

void read_bomb_exploded(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, sizeof(bomb_id_t));
    buf.write_into_buffer(*(bomb_id_t *) buffer);
}

void read_player_moved(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, sizeof(player_id_t));
    buf.write_into_buffer(*(player_id_t *) buffer);
    read_position(buf);
}

void read_block_placed(Buffer &buf) {
    read_position(buf);
}