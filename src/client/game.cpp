#include "game.h"

void Game::add_player(player_id_t id, Player &player) {
    players.insert(make_pair(id, player));
}

void Game::generate_lobby_respond(Buffer &buf) {
    buf.reset_buffer();
    uint8_t msg_code = 0;
    buf.write_into_buffer(msg_code);
    buf.write_into_buffer((uint8_t) server_name.size());
    buf.write_into_buffer(server_name.c_str(), (size_t) server_name.size());
    buf.write_into_buffer(players_count);
    buf.write_into_buffer(htobe16(size_x));
    buf.write_into_buffer(htobe16(size_y));
    buf.write_into_buffer(htobe16(game_length));
    buf.write_into_buffer(htobe16(explosion_radius));
    buf.write_into_buffer(htobe16(bomb_timer));
    auto players_map_size = (uint32_t) players.size();
    buf.write_into_buffer(htobe32(players_map_size));
    for (auto &player: players) {
        buf.write_into_buffer(player.first);
        player.second.generate_respond(buf);
    }
}

void Game::generate_game_respond(Buffer &buf) {
    buf.reset_buffer();
    uint8_t msg_code = 1;
    buf.write_into_buffer(msg_code);
    buf.write_into_buffer((uint8_t) server_name.size());
    buf.write_into_buffer(server_name.c_str(), (size_t) server_name.size());
    buf.write_into_buffer(htobe16(size_x));
    buf.write_into_buffer(htobe16(size_y));
    buf.write_into_buffer(htobe16(game_length));
    buf.write_into_buffer(htobe16(turn));
    auto players_map_size = (uint32_t) players.size();
    cout << "players_map_size: " << players_map_size << endl;
    buf.write_into_buffer(htobe32(players_map_size));
    for (auto &player: players) {
        buf.write_into_buffer(player.first);
        player.second.generate_respond(buf);
    }
    auto players_positions_map_size = (uint32_t) players_positions.size();
    buf.write_into_buffer(htobe32(players_positions_map_size));
    for (auto &player: players_positions) {
        buf.write_into_buffer(player.first);
        buf.write_into_buffer(htobe16(player.second.first));
        buf.write_into_buffer(htobe16(player.second.second));
    }
    auto blocks_list_size = (uint32_t) blocks.size();
    buf.write_into_buffer(htobe32(blocks_list_size));
    for (auto &position: blocks) {
        buf.write_into_buffer(htobe16(position.first));
        buf.write_into_buffer(htobe16(position.second));
    }
    auto bombs_list_size = (uint32_t) bombs.size();
    buf.write_into_buffer(htobe32(bombs_list_size));
    for (auto &bomb: bombs) {
        bomb.second.generate_respond(buf);
    }
    auto explosions_list_size = (uint32_t) explosions.size();
    buf.write_into_buffer(htobe32(explosions_list_size));
    for (auto &position: explosions) {
        buf.write_into_buffer(htobe16(position.first));
        buf.write_into_buffer(htobe16(position.second));
    }
    auto scores_map_size = (uint32_t) scores.size();
    buf.write_into_buffer(htobe32(scores_map_size));
    for (auto &score: scores) {
        buf.write_into_buffer(score.first);
        buf.write_into_buffer(htobe32(score.second));
    }
}

void Game::move_player(Buffer &buf) {
    player_id_t player_id = buf.read_1_byte();
    uint16_t x = be16toh(buf.read_2_bytes());
    uint16_t y = be16toh(buf.read_2_bytes());
    cout << "player_id: " << player_id << endl;
    cout << "x: " << x << endl;
    cout << "y: " << y << endl;
    players_positions.insert(make_pair(player_id, Position(x, y)));
}

void Game::place_block(Buffer &buf) {
    uint16_t x = be16toh(buf.read_2_bytes());
    uint16_t y = be16toh(buf.read_2_bytes());
    blocks.insert(Position(x, y));
}

void Game::place_bomb(Buffer &buf) {

}

void Game::explode_bomb(Buffer &buf) {

}

void Game::reset_game() {

}

void Game::destroy_block(Buffer &buf) {
    uint16_t x = be16toh(buf.read_2_bytes());
    uint16_t y = be16toh(buf.read_2_bytes());
}

void Game::set_turn(turn_t new_turn) {
    this->turn = new_turn;
}

void Bomb::generate_respond(Buffer &buf) const {
    buf.write_into_buffer(pos.first);
    buf.write_into_buffer(pos.second);
    buf.write_into_buffer(timer);
}

//TODO restartowac mape po gameended
