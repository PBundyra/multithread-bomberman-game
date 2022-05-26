#ifndef EVENTS_H
#define EVENTS_H

#include "buffer.h"
#include "utils.h"
#include "game.h"

#define BOMB_PLACED 0
#define BOMB_EXPLODED 1
#define PLAYER_MOVED 2
#define BLOCK_PLACED 3

void read_position(int socket_fd, Buffer &buf);

void read_event(int socket_fd, Buffer &buf, Game &game);

void read_bomb_placed(int socket_fd, Buffer &buf, Game &game);

void read_bomb_exploded(int socket_fd, Buffer &buf, Game &game);

void read_player_moved(int socket_fd, Buffer &buf, Game &game);

void read_block_placed(int socket_fd, Buffer &buf, Game &game);

#endif //EVENTS_H
