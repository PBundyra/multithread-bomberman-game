#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

#include "client.h"
#include "events.h"
#include "player.h"
#include "err.h"


namespace po = boost::program_options;
using namespace std;
using boost::numeric_cast;
using boost::numeric::bad_numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;
using boost::lexical_cast;
using boost::bad_lexical_cast;

enum msg_codes {
    JOIN = 0,
    HELLO = 0,
    ACCEPTED_PLAYER = 1,
    GAME_STARTED = 2,
    TURN = 3,
    GAME_ENDED = 4
};

input_params_t parse_cli_params(int argc, char **argv) {
    input_params_t params;
    po::options_description desc("In order to suppress logging compile with -DNDEBUG option.\nAllowed options");
    desc.add_options()
            ("help,h", "Produce help message")
            ("display-address,d", po::value<string>()->required(), "Set the display address")
            ("player-name,n", po::value<string>()->required(), "Set the player name")
            ("port,p", po::value<string>()->required(), "Set the port")
            ("server-address,s", po::value<string>()->required(), "Set the server address");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }
    po::notify(vm);
    try {
        string server_address = vm["server-address"].as<string>();
        size_t server_port_ind = server_address.find_last_of(':');
        params.server_port = numeric_cast<uint16_t>(lexical_cast<int>(server_address.substr(server_port_ind + 1)));
        params.server_host = server_address.substr(0, server_port_ind);
        params.server_info = tcp::get_addr_info(&params.server_host[0], &server_address[server_port_ind + 1]);

        string display_address = vm["display-address"].as<string>();
        size_t gui_port_ind = display_address.find_last_of(':');
        params.gui_port = numeric_cast<uint16_t>(lexical_cast<int>(display_address.substr(gui_port_ind + 1)));
        params.gui_host = display_address.substr(0, gui_port_ind);
        params.gui_info = udp::get_addr_info(&params.gui_host[0], &display_address[gui_port_ind + 1]);

        params.player_name = vm["player-name"].as<string>();
        params.port = numeric_cast<uint16_t>(lexical_cast<int>(vm["port"].as<string>()));
    } catch (bad_lexical_cast &e) {
        cerr << "Invalid port number" << endl;
        exit(EXIT_FAILURE);
    } catch (positive_overflow &e) {
        cerr << "Port number out of range\n";
        cerr << "Error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (negative_overflow &e) {
        cerr << "Port number out of range\n";
        cerr << "Error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (const bad_numeric_cast &e) {
        cerr << "Error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    }
    return params;
}

size_t Client::get_msg_from_gui() {
    INFO("Waiting for message from GUI");
    errno = 0;
    ssize_t received_length = recv(gui_rec_socket_fd, buf_gui_to_server.get_buffer(), BUFFER_SIZE, 0);
    if (received_length < 0) {
        Err::print_errno();
    }
    INFO("Received " << received_length << " bytes from GUI");
    return (size_t) received_length;
}

void Client::send_msg_to_gui() {
    INFO("Sending message to GUI");
    Buffer::print_buffer(buf_server_to_gui.get_buffer(), buf_server_to_gui.get_no_written_bytes());
    errno = 0;
    ssize_t sent_length = send(gui_send_socket_fd, buf_server_to_gui.get_buffer(),
                               buf_server_to_gui.get_no_written_bytes(), 0);
    INFO("Message to GUI sent");
    if (sent_length < 0) {
        Err::print_errno();
    }
}

void Client::parse_msg_from_gui(const size_t msg_len) {
    INFO("Received message from GUI");
    uint8_t msg_code = buf_gui_to_server.read_1_byte();
    if (is_game_started) {      // checks if the message from gui is valid and  sends PlaceBomb or PlaceBlock or Move
        if (msg_code == 0 && msg_len == 1) {
            INFO("Received PlaceBomb");
            buf_gui_to_server.write_into_buffer((uint8_t) (msg_code + 1));
        } else if (msg_code == 1 && msg_len == 1) {
            INFO("Received PlaceBlock");
            buf_gui_to_server.write_into_buffer((uint8_t) (msg_code + 1));
        } else if (msg_code == 2 && msg_len == 2) {
            INFO("Received Move");
            uint8_t direction = buf_gui_to_server.read_1_byte();
            if (direction < 4) {
                buf_gui_to_server.write_into_buffer((uint8_t) (msg_code + 1));
                buf_gui_to_server.write_into_buffer(direction);
            } else {
                INFO("Invalid direction received");
            }
        } else {
            INFO("Invalid message received");
        }
    } else {        // checks if the message from gui is valid and sends Join
        if ((msg_len == 1 && (msg_code == 0 || msg_code == 1)) || (msg_len == 2 && msg_code == 2)) {
            if (msg_code == 2) {
                uint8_t direction = buf_gui_to_server.read_1_byte();
                if (direction > 3) {
                    INFO("Invalid direction received");
                    return;
                }
            }
            buf_gui_to_server.reset_buffer();
            buf_gui_to_server.write_into_buffer((uint8_t) JOIN);
            buf_gui_to_server.write_into_buffer((uint8_t) player_name.size());
            buf_gui_to_server.write_into_buffer(player_name.c_str(), (size_t) player_name.size());
        }
        INFO("Invalid message received");
    }
}

void Client::send_msg_to_server() {
    if (buf_gui_to_server.get_no_written_bytes() > 0) {
        INFO("Sending message to server");
        errno = 0;
        Buffer::print_buffer(buf_gui_to_server.get_buffer(), buf_gui_to_server.get_no_written_bytes());
        ssize_t sent_length = send(server_socket_fd, buf_gui_to_server.get_buffer(),
                                   buf_gui_to_server.get_no_written_bytes(), 0);
        if (sent_length < 0) {
            Err::print_errno();
        }
        Err::ensure(sent_length == (ssize_t) buf_gui_to_server.get_no_written_bytes());
        INFO("Message to server sent");
    }
}

void Client::handle_hello_msg(Buffer &buf) {
    char local_buf[sizeof(uint16_t)];
    read_str(server_socket_fd, buf);                         // server name
//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint8_t));
//    buf.write_into_buffer(*(uint8_t *) local_buf);              // players count
    buf.write_into_buffer(get_uint8_t_from_server(server_socket_fd));              // players count

//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint16_t));
//    buf.write_into_buffer(*(uint16_t *) local_buf);             // size x
    buf.write_into_buffer(get_uint16_t_from_server(server_socket_fd));             // size x

//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint16_t));
//    buf.write_into_buffer(*(uint16_t *) local_buf);             // size y
    buf.write_into_buffer(get_uint16_t_from_server(server_socket_fd));             // size x

//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint16_t));
//    buf.write_into_buffer(*(uint16_t *) local_buf);             // game length
    buf.write_into_buffer(get_uint16_t_from_server(server_socket_fd));             // size x

//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint16_t));
//    buf.write_into_buffer(*(uint16_t *) local_buf);             // explosion radius
    buf.write_into_buffer(get_uint16_t_from_server(server_socket_fd));             // size x

//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint16_t));
//    buf.write_into_buffer(*(uint16_t *) local_buf);             // bomb timer
    buf.write_into_buffer(get_uint16_t_from_server(server_socket_fd));             // size x

    game = Game(buf);
    buf.reset_buffer();
    game.serialize_lobby_respond(buf);
}

void Client::handle_accepted_player_msg(Buffer &buf) {
    Player::read_player(server_socket_fd, buf);
    player_id_t player_id = buf_server_to_gui.read_1_byte();
    Player player(buf_server_to_gui);
    game.add_player(player_id, player);
    buf.reset_buffer();
    game.serialize_lobby_respond(buf);
}

void Client::handle_game_started_msg(Buffer &buf) {
//    char buffer[sizeof(uint32_t)];
//    get_n_bytes_from_server(server_socket_fd, buffer, sizeof(uint32_t));
//    uint32_t map_len = be32toh(*(uint32_t *) buffer);
    uint32_t map_len = get_uint32_t_from_server(server_socket_fd);
    for (uint32_t i = 0; i < map_len; i++) {
        buf.reset_buffer();
        handle_accepted_player_msg(buf);
    }
    is_game_started = true;
}

void Client::handle_turn_msg(Buffer &buf) {
//    char local_buf[sizeof(uint32_t)];
//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(turn_t));
//    turn_t turn = be16toh(*(turn_t *) local_buf);
    uint16_t turn = get_uint16_t_from_server(server_socket_fd);
    game.set_turn(turn);
//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint32_t));
//    uint32_t list_len = be32toh(*(uint32_t *) local_buf);
    uint32_t list_len = get_uint32_t_from_server(server_socket_fd);
    for (uint32_t i = 0; i < list_len; i++) {
        deserialize_event(server_socket_fd, buf, game);
    }
    game.add_scores();
    game.erase_destroyed_blocks();
    game.serialize_game_respond(buf);
    game.reset_turn();
}

void Client::handle_game_ended_msg(Buffer &buf) {
//    char local_buf[sizeof(uint32_t)];
//    get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(uint32_t));
//    uint32_t map_len = be32toh(*(uint32_t *) local_buf);
    uint32_t map_len = get_uint32_t_from_server(server_socket_fd);
    for (uint32_t i = 0; i < map_len; i++) {
        get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(player_id_t));
        get_n_bytes_from_server(server_socket_fd, local_buf, sizeof(score_t));
    }
    is_game_started = false;
    buf.reset_buffer();
    game.reset_game();
    game.serialize_lobby_respond(buf);
}

[[noreturn]] void Client::gui_to_server_handler() {
    size_t msg_len;
    do {
        buf_gui_to_server.reset_buffer();
        msg_len = get_msg_from_gui();
        parse_msg_from_gui(msg_len);
        send_msg_to_server();
    } while (true);
}

[[noreturn]] void Client::server_to_gui_handler() {
    char local_buf[1];
    do {
        buf_server_to_gui.reset_buffer();
        INFO("Waiting for message from server");
        get_n_bytes_from_server(server_socket_fd, local_buf, 1);
        INFO("Received message from server");
        switch (local_buf[0]) {
            case HELLO:
                INFO("Received Hello from server");
                handle_hello_msg(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case ACCEPTED_PLAYER:
                INFO("Received Accepted Player from server");
                handle_accepted_player_msg(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case GAME_STARTED:
                INFO("Received Game Started from server");
                handle_game_started_msg(buf_server_to_gui);
                break;
            case TURN:
                INFO("Received Turn from server");
                handle_turn_msg(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case GAME_ENDED:
                INFO("Received Game Ended from server");
                handle_game_ended_msg(buf_server_to_gui);
                send_msg_to_gui();
                break;
            default:
                Err::fatal("Received unknown message from server");
                break;
        }
    } while (true);
}

void Client::run() {
    thread gui_to_server_thread([this] { gui_to_server_handler(); });
    thread server_to_gui_thread([this] { server_to_gui_handler(); });
    gui_to_server_thread.join();
    server_to_gui_thread.join();
}

int main(int argc, char **argv) {
    input_params_t input_params = parse_cli_params(argc, argv);
    Client client = Client(input_params);
    client.run();
    return 0;
}
