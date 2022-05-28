#ifndef EVENTS_H
#define EVENTS_H

#include "buffer.h"
#include "utils.h"
#include "game.h"

#define BOMB_PLACED 0
#define BOMB_EXPLODED 1
#define PLAYER_MOVED 2
#define BLOCK_PLACED 3


class Event {
private:
    static void deserialize_bomb_placed(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr);

    static void deserialize_bomb_exploded(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr);

    static void deserialize_player_moved(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr);

    static void deserialize_block_placed(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr);

public:
    static void deserialize_position(int socket_fd, Buffer &buf, struct sockaddr_in6 addr);

    static void deserialize_event(int socket_fd, Buffer &buf, Game &game, struct sockaddr_in6 addr);
};

#endif //EVENTS_H
