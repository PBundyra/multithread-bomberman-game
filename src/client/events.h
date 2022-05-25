#ifndef EVENTS_H
#define EVENTS_H

using player_id_t = uint8_t;
using bomb_id_t = uint32_t;

class Event {};

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
