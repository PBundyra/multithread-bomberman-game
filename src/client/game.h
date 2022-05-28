#ifndef GAME_H
#define GAME_H

#include <memory>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "utils.h"
#include "player.h"
#include "buffer.h"

using score_t = uint32_t;
using turn_t = uint16_t;
using bomb_id_t = uint32_t;

struct Bomb {
    Position pos;
    uint16_t timer;

    Bomb(Position pos, uint16_t timer) : pos(pos), timer(timer) {};

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

        std::cout << "Server name: " << server_name << std::endl;
        std::cout << "No players: " << (int) players_count << std::endl;
        std::cout << "Size X: " << size_x << std::endl;
        std::cout << "Size Y: " << size_y << std::endl;
        std::cout << "Game length: " << game_length << std::endl;
        std::cout << "Explosion radius: " << explosion_radius << std::endl;
        std::cout << "Bomb timer: " << bomb_timer << std::endl;
    };

    void generate_lobby_respond(Buffer &buf);

    void generate_game_respond(Buffer &buf);

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
