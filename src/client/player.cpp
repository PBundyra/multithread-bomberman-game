#include "player.h"

//void read_player(Buffer &buf) {
//    char buffer[1];
//    get_n_bytes_from_server(buffer, 1);
//    buf.write_into_buffer(buffer[0]);               // player id
//    read_str(buf);                                  // player name
//    read_str(buf);                                  // player address
//}
void Player::generate_respond(Buffer &buf){
    cout << "Sending player " << name << " | " << addr << endl;
    buf.write_into_buffer((uint8_t) name.size());
    buf.write_into_buffer(name.c_str(), (size_t) name.size());
    buf.write_into_buffer((uint8_t) addr.size());
    buf.write_into_buffer(addr.c_str(), (size_t) addr.size());
}
