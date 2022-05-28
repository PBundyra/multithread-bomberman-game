#include "client.h"

namespace po = boost::program_options;
namespace asio = boost::asio;
namespace ip = asio::ip;
using namespace std;
using boost::numeric_cast;
using boost::numeric::bad_numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;
using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::ip::address;

#define JOIN 0
#define HELLO 0
#define ACCEPTED_PLAYER 1
#define GAME_STARTED 2
#define TURN 3
#define GAME_ENDED 4


input_params_t parse_cli_params(int argc, char **argv) {
    input_params_t params;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("display-address,d", po::value<string>()->required(), "Set the display address")
            ("player-name,n", po::value<string>()->required(), "Set the player name")
            ("port,p", po::value<string>()->required(), "Set the port")
            ("server-address,s", po::value<string>()->required(), "Set the server address");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        exit(0);
    }
    po::notify(vm);
    try {
        string display_address = vm["display-address"].as<string>();
        string server_address = vm["server-address"].as<string>();
        size_t server_port_ind = server_address.find_last_of(':');
        params.server_port = numeric_cast<port_t>(lexical_cast<int>(server_address.substr(server_port_ind + 1)));
        params.server_host = server_address.substr(0, server_port_ind);
        params.server_addr = get_address(&params.server_host[0], params.server_port);
        size_t gui_port_ind = display_address.find_last_of(':');
        params.gui_port = numeric_cast<port_t>(lexical_cast<int>(display_address.substr(gui_port_ind + 1)));
        params.gui_host = display_address.substr(0, gui_port_ind);
        params.gui_addr = get_send_address(&params.gui_host[0], params.gui_port);
        params.player_name = vm["player-name"].as<string>();
        params.port = numeric_cast<port_t>(lexical_cast<int>(vm["port"].as<string>()));
    } catch (bad_lexical_cast &e) {
        cerr << "Invalid port number" << endl;
        exit(1);
    } catch (positive_overflow &e) {
        cerr << "Port number out of range\n";
        cerr << "Error: " << e.what() << "\n";
        exit(1);
    } catch (negative_overflow &e) {
        cerr << "Port number out of range\n";
        cerr << "Error: " << e.what() << "\n";
        exit(1);
    } catch (const bad_numeric_cast &e) {
        cerr << "Error: " << e.what() << "\n";
        exit(1);
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << "\n";
        exit(1);
    }
    return params;
}

size_t Client::get_msg_from_gui() {
    auto address_length = (socklen_t) sizeof(gui_addr);
    errno = 0;
    ssize_t received_length = recvfrom(udp_socket_fd, buf_gui_to_server.get_buffer(), BUFFER_SIZE,
                                       0,
                                       (struct sockaddr *) &gui_addr,
                                       &address_length);
    if (received_length < 0) {
        PRINT_ERRNO();
    }
    return (size_t) received_length;
}

void Client::send_msg_to_gui() {
    auto address_length = (socklen_t) sizeof(gui_addr);
    INFO("Sending message to GUI");
    Buffer::print_buffer(buf_server_to_gui.get_buffer(), buf_server_to_gui.get_no_written_bytes());
    ssize_t sent_length = sendto(udp_socket_fd, buf_server_to_gui.get_buffer(),
                                 buf_server_to_gui.get_no_written_bytes(), 0,
                                 (struct sockaddr *) &gui_addr, address_length);
    INFO("Message to GUI sent");
    ENSURE(sent_length == (ssize_t) buf_server_to_gui.get_no_written_bytes());
}

void Client::parse_msg_from_gui(const size_t msg_len) {
    ENSURE(msg_len == 2 || msg_len == 1);
    INFO("Received message from GUI");
    if (is_game_started) {      // sends PlaceBomb or PlaceBlock or Move
        uint8_t msg_code = buf_gui_to_server.read_1_byte();
        if (msg_code == 2) {
            INFO("Received MOVE");
            uint8_t direction = buf_gui_to_server.read_1_byte();
            buf_gui_to_server.write_into_buffer((uint8_t) (msg_code + 1));
            buf_gui_to_server.write_into_buffer(direction);
        } else {
            INFO("Received PlaceBomb or PlaceBlock");
            buf_gui_to_server.write_into_buffer((uint8_t) (msg_code + 1));
        }
    } else {    // sends Join
        INFO("Game not started");
        buf_gui_to_server.reset_buffer();
        buf_gui_to_server.write_into_buffer((uint8_t) JOIN);
        buf_gui_to_server.write_into_buffer((uint8_t) player_name.size());
        buf_gui_to_server.write_into_buffer(player_name.c_str(), (size_t) player_name.size());
        INFO("Player name:" << player_name);
    }
    INFO("Message to server parsed");
}

size_t Client::get_n_bytes_from_server(void *buffer, const size_t n) const {
    errno = 0;
    ssize_t received_length = recv(tcp_socket_fd, buffer, n, MSG_WAITALL);
    if (received_length < 0) {
        PRINT_ERRNO();
    } else if (received_length == 0) {
        INFO("Server closed connection");
        exit(0);
    }
    INFO ("Received " << received_length << " bytes from server");
    return (size_t) received_length;
}

void Client::send_msg_to_server() {
    INFO("Sending message to server");
    errno = 0;
    Buffer::print_buffer(buf_gui_to_server.get_buffer(), buf_gui_to_server.get_no_written_bytes());
    ssize_t sent_length = send(tcp_socket_fd, buf_gui_to_server.get_buffer(),
                               buf_gui_to_server.get_no_written_bytes(), 0);
    if (sent_length < 0) {
        PRINT_ERRNO();
    }
    ENSURE(sent_length == (ssize_t) buf_gui_to_server.get_no_written_bytes());
    INFO("Message to server sent");
}

void Client::read_hello(Buffer &buf) {
    char local_buf[sizeof(uint16_t)];
    read_str(tcp_socket_fd, buf);                         // server name
    get_n_bytes_from_server(local_buf, sizeof(uint8_t));
    buf.write_into_buffer(*(uint8_t *) local_buf);              // players count
    get_n_bytes_from_server(local_buf, sizeof(uint16_t));
    buf.write_into_buffer(*(uint16_t *) local_buf);             // size x
    get_n_bytes_from_server(local_buf, sizeof(uint16_t));
    buf.write_into_buffer(*(uint16_t *) local_buf);             // size y
    get_n_bytes_from_server(local_buf, sizeof(uint16_t));
    buf.write_into_buffer(*(uint16_t *) local_buf);             // game length
    get_n_bytes_from_server(local_buf, sizeof(uint16_t));
    buf.write_into_buffer(*(uint16_t *) local_buf);             // explosion radius
    get_n_bytes_from_server(local_buf, sizeof(uint16_t));
    buf.write_into_buffer(*(uint16_t *) local_buf);             // bomb timer
    game = Game(buf);
    buf.reset_buffer();
    game.serialize_lobby_respond(buf);
    send_msg_to_gui();
}

void Client::read_accepted_player(Buffer &buf) {
    Player::read_player(tcp_socket_fd, buf);
    player_id_t player_id = buf_server_to_gui.read_1_byte();
    Player player(buf_server_to_gui);
    game.add_player(player_id, player);
    buf.reset_buffer();
    game.serialize_lobby_respond(buf);
}

void Client::read_game_started(Buffer &buf) {
    char buffer[sizeof(map_len_t)];
    get_n_bytes_from_server(buffer, sizeof(map_len_t));
    map_len_t map_len = be32toh(*(map_len_t *) buffer);
    for (map_len_t i = 0; i < map_len; i++) {
        buf.reset_buffer();
        read_accepted_player(buf);
    }
    is_game_started = true;
}

void Client::read_turn(Buffer &buf) {
    char local_buf[sizeof(list_len_t)];
    get_n_bytes_from_server(local_buf, sizeof(turn_t));
    turn_t turn = be16toh(*(turn_t *) local_buf);
    game.set_turn(turn);
    get_n_bytes_from_server(local_buf, sizeof(list_len_t));
    list_len_t list_len = be32toh(*(list_len_t *) local_buf);
    for (list_len_t i = 0; i < list_len; i++) {
        deserialize_event(tcp_socket_fd, buf, game);
    }
    game.add_scores();
    game.erase_destroyed_blocks();
    game.serialize_game_respond(buf);
    game.reset_turn();
    send_msg_to_gui();
}

void Client::read_game_ended(Buffer &buf) {
    char local_buf[sizeof(map_len_t)];
    get_n_bytes_from_server(local_buf, sizeof(map_len_t));
    map_len_t map_len = be32toh(*(map_len_t *) local_buf);
    for (map_len_t i = 0; i < map_len; i++) {
        get_n_bytes_from_server(local_buf, sizeof(player_id_t));
        get_n_bytes_from_server(local_buf, sizeof(score_t));
    }
    is_game_started = false;
    buf.reset_buffer();
    game.reset_game();
    game.serialize_lobby_respond(buf);
    send_msg_to_gui();
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
        get_n_bytes_from_server(local_buf, 1);
        INFO("Received message from server");
        switch (local_buf[0]) {
            case HELLO:
                INFO("Received HELLO from server");
                read_hello(buf_server_to_gui);
                break;
            case ACCEPTED_PLAYER:
                INFO("Received ACCEPTED_PLAYER from server");
                read_accepted_player(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case GAME_STARTED:
                INFO("Received GAME_STARTED from server");
                read_game_started(buf_server_to_gui);
                break;
            case TURN:
                INFO("Received TURN from server");
                read_turn(buf_server_to_gui);
                break;
            case GAME_ENDED:
                INFO("Received GAME_ENDED from server");
                read_game_ended(buf_server_to_gui);
                break;
            default:
                fatal("Received unknown message from server");
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
