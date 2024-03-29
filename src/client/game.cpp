#include <utility>
#include <memory>

#include "game.h"


void Bomb::serialize_bomb(Buffer &buf) const {
    buf.write_into_buffer(htobe16(pos.first));
    buf.write_into_buffer(htobe16(pos.second));
    buf.write_into_buffer(htobe16(timer));
}

void Game::serialize_players(Buffer &buf) {
    auto players_map_size = (map_len_t) players.size();
    buf.write_into_buffer(htobe32(players_map_size));
    for (auto &player: players) {
        buf.write_into_buffer(player.first);
        player.second.serialize_player(buf);
    }
}

void Game::serialize_players_positions(Buffer &buf) {
    auto players_positions_map_size = (map_len_t) players_positions.size();
    buf.write_into_buffer(htobe32(players_positions_map_size));
    for (auto &player: players_positions) {
        buf.write_into_buffer(player.first);
        buf.write_into_buffer(htobe16(player.second.first));
        buf.write_into_buffer(htobe16(player.second.second));
    }
}

void Game::serialize_blocks(Buffer &buf) {
    auto blocks_list_len = (list_len_t) blocks.size();
    buf.write_into_buffer(htobe32(blocks_list_len));
    for (auto &position: blocks) {
        buf.write_into_buffer(htobe16(position.first));
        buf.write_into_buffer(htobe16(position.second));
    }
}

void Game::serialize_bombs(Buffer &buf) {
    auto bombs_list_len = (list_len_t) bombs.size();
    buf.write_into_buffer(htobe32(bombs_list_len));
    for (auto &bomb: bombs) {
        bomb.second.serialize_bomb(buf);
    }
}

void Game::serialize_explosions(Buffer &buf) {
    auto explosions_list_len = (list_len_t) explosions.size();
    buf.write_into_buffer(htobe32(explosions_list_len));
    for (auto &position: explosions) {
        buf.write_into_buffer(htobe16(position.first));
        buf.write_into_buffer(htobe16(position.second));
    }
}

void Game::serialize_scores(Buffer &buf) {
    auto scores_map_len = (map_len_t) scores.size();
    buf.write_into_buffer(htobe32(scores_map_len));
    for (auto &score: scores) {
        buf.write_into_buffer(score.first);
        buf.write_into_buffer(htobe32(score.second));
    }
}

void Game::add_player(player_id_t id, Player &player) {
    players.insert(std::make_pair(id, player));
    scores.insert(std::make_pair(id, 0));
}

void Game::serialize_lobby_respond(Buffer &buf) {
    buf.reset_buffer();
    uint8_t msg_code = 0;
    buf.write_into_buffer(msg_code);
    buf.write_into_buffer((str_len_t) server_name.size());
    buf.write_into_buffer(server_name.c_str(), (str_len_t) server_name.size());
    buf.write_into_buffer(players_count);
    buf.write_into_buffer(htobe16(size_x));
    buf.write_into_buffer(htobe16(size_y));
    buf.write_into_buffer(htobe16(game_length));
    buf.write_into_buffer(htobe16(explosion_radius));
    buf.write_into_buffer(htobe16(bomb_timer));
    serialize_players(buf);
}

void Game::serialize_game_respond(Buffer &buf) {
    buf.reset_buffer();
    uint8_t msg_code = 1;
    buf.write_into_buffer(msg_code);
    buf.write_into_buffer((str_len_t) server_name.size());
    buf.write_into_buffer(server_name.c_str(), (str_len_t) server_name.size());
    buf.write_into_buffer(htobe16(size_x));
    buf.write_into_buffer(htobe16(size_y));
    buf.write_into_buffer(htobe16(game_length));
    buf.write_into_buffer(htobe16(turn));
    serialize_players(buf);
    serialize_players_positions(buf);
    serialize_blocks(buf);
    serialize_bombs(buf);
    serialize_explosions(buf);
    serialize_scores(buf);
}

void Game::move_player(Buffer &buf) {
    player_id_t player_id = buf.read_1_byte();
    cord_t x = buf.read_2_bytes();
    cord_t y = buf.read_2_bytes();
    INFO("Player " << player_id << " moved to (" << x << ", " << y << ")");
    players_positions[player_id] = {x, y};
}

void Game::kill_player(Buffer &buf) {
    player_id_t player_id = buf.read_1_byte();
    dead_players.insert(player_id);
}

void Game::place_block(Buffer &buf) {
    cord_t x = buf.read_2_bytes();
    cord_t y = buf.read_2_bytes();
    INFO("Block placed at (" << x << ", " << y << ")");
    blocks.insert(Position(x, y));
}

void Game::place_bomb(Buffer &buf) {
    bomb_id_t bomb_id = buf.read_4_bytes();
    cord_t x = buf.read_2_bytes();
    cord_t y = buf.read_2_bytes();
    Position position(x, y);
    Bomb bomb(position, bomb_timer);
    INFO("Bomb " << bomb_id << " placed at (" << x << ", " << y << ")");
    bombs.insert(std::make_pair(bomb_id, bomb));
}

void Game::explode_bomb(Buffer &buf) {
    bomb_id_t bomb_id = buf.read_4_bytes();
    Bomb bomb = bombs.at(bomb_id);
    cord_t x = bomb.pos.first;
    cord_t y = bomb.pos.second;
    INFO("Bomb " << bomb_id << " exploded at (" << x << ", " << y << ")");

    for (cord_t i = 0; i <= explosion_radius; i++) {
        if (x + i < size_x) {
            explosions.insert(std::make_pair(x + i, y));
            if (blocks.find(Position(x + i, y)) != blocks.end()) {
                break;
            }
        }
    }
    for (cord_t i = 0; i <= explosion_radius; i++) {
        if (x - i >= 0) {
            explosions.insert(Position(x - i, y));
            if (blocks.find(Position(x - i, y)) != blocks.end()) {
                break;
            }
        }
    }
    for (cord_t i = 0; i <= explosion_radius; i++) {
        if (y + i < size_y) {
            explosions.insert(Position(x, y + i));
            if (blocks.find(Position(x, y + i)) != blocks.end()) {
                break;
            }
        }
    }
    for (cord_t i = 0; i <= explosion_radius; i++) {
        if (y - i >= 0) {
            explosions.insert(Position(x, y - i));
            if (blocks.find(Position(x, y - i)) != blocks.end()) {
                break;
            }
        }
    }
    explosions.insert(bomb.pos);
    bombs.erase(bomb_id);
    for (auto &explosion: explosions) {
        INFO("Explosion at (" << explosion.first << ", " << explosion.second << ")");
    }
}

void Game::reset_game() {
    players.clear();
    players_positions.clear();
    blocks.clear();
    bombs.clear();
    explosions.clear();
    scores.clear();
}

void Game::add_scores() {
    for (auto player_id: dead_players) {
        scores[player_id] += 1;
    }
}

void Game::reset_turn() {
    dead_players.clear();
    explosions.clear();
    destroyed_blocks.clear();
    for (auto &bomb: bombs) {
        bomb.second.timer -= 1;
    }
}

void Game::destroy_block(Buffer &buf) {
    uint16_t x = buf.read_2_bytes();
    uint16_t y = buf.read_2_bytes();
    destroyed_blocks.insert(Position(x, y));
}

void Game::erase_destroyed_blocks() {
    for (auto &pos: destroyed_blocks) {
        blocks.erase(pos);
    }
}

void Game::set_turn(turn_t new_turn) {
    this->turn = new_turn;
}
