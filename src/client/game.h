#ifndef GAME_H
#define GAME_H

#include <memory>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <utility>

#include "utils.h"
#include "player.h"
#include "buffer.h"

using score_t = uint32_t;
using turn_t = uint16_t;
using bomb_id_t = uint32_t;

struct Bomb {
    Position pos;
    uint16_t timer;

    Bomb(Position pos, uint16_t timer) : pos(std::move(pos)), timer(timer) {};

    void generate_respond(Buffer &buf) const;
};

class Game {
private:
    std::string server_name;
    uint8_t players_count{};
    uint16_t size_x{};
    uint16_t size_y{};
    uint16_t game_length{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};
    uint16_t turn{};
    std::unordered_map<player_id_t, Player> players{};
    std::unordered_map<player_id_t, Position> players_positions{};
    std::set<Position> blocks;
    std::unordered_map<bomb_id_t, Bomb> bombs;
    std::set<Position> explosions;
    std::unordered_map<player_id_t, score_t> scores;
    std::set<player_id_t> dead_players;
    std::set<Position> destroyed_blocks;

public:
    Game() = default;

    explicit Game(Buffer &buffer) {
        uint8_t server_name_length = buffer.read_1_byte();
        server_name = buffer.read_n_bytes(server_name_length);
        players_count = buffer.read_1_byte();
        size_x = buffer.read_2_bytes();
        size_y = buffer.read_2_bytes();
        game_length = buffer.read_2_bytes();
        explosion_radius = buffer.read_2_bytes();
        bomb_timer = buffer.read_2_bytes();

        INFO("Server name: " << server_name);
        INFO("Players count: " << (int) players_count);
        INFO("Size x: " << size_x);
        INFO("Size y: " << size_y);
        INFO("Game length: " << game_length);
        INFO("Explosion radius: " << explosion_radius);
        INFO("Bomb timer: " << bomb_timer);
    };

    void serialize_lobby_respond(Buffer &buf);

    void serialize_game_respond(Buffer &buf);

    void add_player(player_id_t id, Player &player);

    void move_player(Buffer &buf);

    void kill_player(Buffer &buf);

    void place_block(Buffer &buf);

    void destroy_block(Buffer &buf);

    void erase_blocks();

    void place_bomb(Buffer &buf);

    void explode_bomb(Buffer &buf);

    void reset_game();

    void add_scores();

    void reset_turn();

    void set_turn(turn_t new_turn);
};

#endif //GAME_H
