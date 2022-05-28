#ifndef EVENTS_H
#define EVENTS_H

#include "buffer.h"
#include "utils.h"
#include "game.h"

#define BOMB_PLACED 0
#define BOMB_EXPLODED 1
#define PLAYER_MOVED 2
#define BLOCK_PLACED 3

// Receives a message with a position from the server and writes it to the buffer
void read_position(int socket_fd, Buffer &buf);

// Receives a message with an event from the server, processes it and makes accordingly the changes in the game
void deserialize_event(int socket_fd, Buffer &buf, Game &game);

#endif //EVENTS_H
