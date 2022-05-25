#include "player.h"

void read_player(Buffer &buf) {
    char buffer[1];
    get_n_bytes_from_server(buffer, 1);
    buf.write_into_buffer(buffer[0]);               // player id
    read_str(buf);                                  // player name
    read_str(buf);                                  // player address
}