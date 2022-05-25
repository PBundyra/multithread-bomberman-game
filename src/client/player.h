#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "buffer.h"

namespace asio = boost::asio;
using namespace std;

class Player {
private:
    string name;
    string addr;
public:
    Player(const Buffer &buf) {
        name = buf.read_n_bytes((size_t) buf.read_1_byte());
        addr = buf.read_n_bytes((size_t) buf.read_1_byte());
    }

}

void read_player(Buffer &buf);

#endif //PLAYER_H
