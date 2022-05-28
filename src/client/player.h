#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "buffer.h"
#include "utils.h"

using player_id_t = uint8_t;

class Player {
private:
    std::string name;
    std::string addr;
public:
    explicit Player(Buffer &buf) {
        name = buf.read_n_bytes((size_t) buf.read_1_byte());
        addr = buf.read_n_bytes((size_t) buf.read_1_byte());

        INFO("Created player << " << name << " with address " << addr << ".");
    }

    void generate_respond(Buffer &buf);
};

void deserialize_player(int socket_fd, Buffer &buf, struct sockaddr_in6 addr);

#endif //PLAYER_H
