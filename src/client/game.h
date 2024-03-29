#ifndef GAME_H
#define GAME_H

#include <unordered_map>
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

    Bomb(Position pos, uint16_t timer) : pos(std::move(pos)), timer(timer) {};

    // Serializes the bomb into a buffer.
    void serialize_bomb(Buffer &buf) const;
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

    void serialize_players(Buffer &buf);

    void serialize_players_positions(Buffer &buf);

    void serialize_blocks(Buffer &buf);

    void serialize_bombs(Buffer &buf);

    void serialize_explosions(Buffer &buf);

    void serialize_scores(Buffer &buf);

public:
    Game() = default;

    explicit Game(Buffer &buffer) : turn(0) {
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

    // Serializes a lobby state to a buffer
    void serialize_lobby_respond(Buffer &buf);

    // Serializes a game state to a buffer
    void serialize_game_respond(Buffer &buf);

    void add_player(player_id_t id, Player &player);

    void move_player(Buffer &buf);

    // Adds a player to the dead players set.
    void kill_player(Buffer &buf);

    void place_block(Buffer &buf);

    // Adds a block to the destroyed blocks set.
    void destroy_block(Buffer &buf);

    // Erases blocks that are in the destroyed blocks set.
    void erase_destroyed_blocks();

    void place_bomb(Buffer &buf);

    void explode_bomb(Buffer &buf);

    // Resets the game to the lobby state.
    void reset_game();

    // Increases score of players that are in the dead players set.
    void add_scores();

    // Reset turn to the state without explosions, dead players and destroyed blocks.
    void reset_turn();

    // Sets the turn number.
    void set_turn(turn_t new_turn);
};

#endif //GAME_H
