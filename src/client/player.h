#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "buffer.h"

using namespace std;

class Player {
private:
    string name;
    string addr;
public:
    Player(Buffer &buf) {
        name = buf.read_n_bytes((size_t) buf.read_1_byte());
        addr = buf.read_n_bytes((size_t) buf.read_1_byte());
        cout << "Created player " << name << " with address " << addr << endl;
    }

    void generate_respond(Buffer &buf);
};

void read_player(Buffer &buf);

#endif //PLAYER_H
