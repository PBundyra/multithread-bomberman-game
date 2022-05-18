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
    uint16_t size_x;
    uint16_t size_y;
    uint16_t game_length;
    uint16_t turn;
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

    Map(const string &server_name, uint16_t size_x, uint16_t size_y, uint16_t game_length) : server_name(server_name), size_x(size_x),
                                                                size_y(size_y), game_length(game_length) {
        cout << "Map created" << endl;
        cout << "Server name: " << server_name << endl;
    };

    void apply_changes(const vector<shared_ptr<Event>> &events);

    void generate_respond(Buffer &buf);

    void add_player(player_id_t id, Player &player);


};


#endif //MAP_H
