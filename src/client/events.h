#ifndef EVENTS_H
#define EVENTS_H

#include "buffer.h"
#include "utils.h"

#define BOMB_PLACED 0
#define BOMB_EXPLODED 1
#define PLAYER_MOVED 2
#define BLOCK_PLACED 3

using player_id_t = uint8_t;
using bomb_id_t = uint32_t;

void read_event(Buffer &buf);

//void read_bomb_placed(Buffer &buf);
//
//void read_bomb_exploded(Buffer &buf);
//
//void read_player_moved(Buffer &buf);
//
//void read_block_placed(Buffer &buf);


class Event {
private:

};

class BombPlaced : public Event {
private:
    bomb_id_t id;
};

class BombExploded : public Event {
};

class PlayerMoved : public Event {
};

class BlockPlaced : public Event {
};


#endif //EVENTS_H
