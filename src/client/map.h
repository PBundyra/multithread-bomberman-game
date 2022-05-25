#ifndef MAP_H
#define MAP_H

#include <memory>
#include <iostream>

#include "utils.h"
#include "events.h"
#include "player.h"
#include "buffer.h"

using namespace std;

using score_t = uint32_t;

class Map {
private:
    string server_name;
    uint8_t players_count{};
    uint16_t size_x{};
    uint16_t size_y{};
    uint16_t game_length{};
    uint16_t explosion_radius{};
    uint16_t bomb_timer{};
    uint16_t turn{};
    unordered_map<player_id_t, shared_ptr<Player>> players;
    unordered_map<player_id_t, shared_ptr<Position>> player_positions;
    vector<shared_ptr<Position>> blocks;
    unordered_map<bomb_id_t, shared_ptr<Position>> bombs;
    vector<shared_ptr<Position>> explosions;
    unordered_map<player_id_t, score_t> scores;

    void apply_changes(unique_ptr<BombPlaced> event);

    void apply_changes(unique_ptr<BombExploded> event);

    void apply_changes(unique_ptr<PlayerMoved> event);

    void apply_changes(unique_ptr<BlockPlaced> event);

public:
    Map() = default;

    explicit Map(Buffer &buffer) {
        uint8_t server_name_length = buffer.read_1_byte();
        server_name = buffer.read_n_bytes(server_name_length);
        players_count = buffer.read_1_byte();
        size_x = buffer.read_2_bytes();
        size_y = buffer.read_2_bytes();
        game_length = buffer.read_2_bytes();
        explosion_radius = buffer.read_2_bytes();
        bomb_timer = buffer.read_2_bytes();

        cout << "Server name: " << server_name << endl;
        cout << "No players: " << (int)players_count << endl;
        cout << "Size X: " << size_x << endl;
        cout << "Size Y: " << size_y << endl;
        cout << "Game length: " << game_length << endl;
        cout << "Explosion radius: " << explosion_radius << endl;
        cout << "Bomb timer: " << bomb_timer << endl;
//        cout << "Explosion length: " <<  << endl;
    };

    void apply_changes(const vector<shared_ptr<Event>> &events);

    void generate_respond(Buffer &buf);

    void add_player(player_id_t id, Player &player);


};


#endif //MAP_H
