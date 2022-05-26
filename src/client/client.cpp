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
    cout << "Sending to GUI" << endl;
    ssize_t sent_length = sendto(udp_socket_fd, buf_server_to_gui.get_buffer(),
                                 buf_server_to_gui.get_no_written_bytes(), 0,
                                 (struct sockaddr *) &gui_addr, address_length);
    cout << "Send " << sent_length << " bytes to GUI" << endl;
    buf_server_to_gui.print_buffer(buf_server_to_gui.get_buffer(), sent_length);
    ENSURE(sent_length == (ssize_t) buf_server_to_gui.get_no_written_bytes());
}

void Client::parse_msg_from_gui(const size_t msg_len) {
    ENSURE(msg_len == 2 || msg_len == 1);
    cout << "Received message from GUI of length " << msg_len << endl;
    if (is_game_started) {  // sends PlaceBomb or PlaceBlock or Move
        uint8_t msg_code = buf_gui_to_server.read_1_byte();
        cout << "Sending " << (int) (msg_code + 1) << endl;
        buf_gui_to_server.overwrite_buffer((uint8_t) (msg_code + 1), (size_t) 0);
        buf_gui_to_server.print_buffer(buf_gui_to_server.get_buffer(), msg_len);
    } else {    // sends Join
        cout << "Sending Join" << endl;
        buf_gui_to_server.reset_buffer();
        buf_gui_to_server.write_into_buffer((uint8_t) JOIN);
        buf_gui_to_server.write_into_buffer((uint8_t) player_name.size());
        buf_gui_to_server.write_into_buffer(player_name.c_str(), player_name.size());
    }
}

size_t Client::get_n_bytes_from_server(void *buffer, const size_t n) const {
    errno = 0;
    ssize_t received_length = recv(tcp_socket_fd, buffer, n, MSG_WAITALL);
    if (received_length < 0) {
        PRINT_ERRNO();
    } else if (received_length == 0) {
        cout << "Server closed connection" << endl;
        exit(0);
    }
    std::cout << "Received message from server of length: " << received_length << "\n";
    return (size_t) received_length;
}

void Client::send_msg_to_server(const size_t msg_len) {
    cout << "Sending message to server of length " << msg_len << endl;
    errno = 0;
    ssize_t sent_length = send(tcp_socket_fd, buf_gui_to_server.get_buffer(),
                               msg_len, 0);
    if (sent_length < 0) {
        PRINT_ERRNO();
    }
    ENSURE(sent_length == (ssize_t) msg_len);
    cout << "Sent message to server" << endl;
}

void Client::gui_to_server_handler() {
    size_t msg_len;
    do {
        buf_gui_to_server.reset_buffer();
        msg_len = get_msg_from_gui();
        parse_msg_from_gui(msg_len);
        send_msg_to_server(msg_len);
    } while (msg_len != 0);
}

void Client::server_to_gui_handler() {
    char buffer[1];
    size_t msg_len;
    do {
//    for (int i = 0; i < 6; i++) {
        buf_server_to_gui.reset_buffer();
        msg_len = get_n_bytes_from_server(buffer, 1);
//        buf_server_to_gui.write_into_buffer((uint8_t) buffer[0]);
        switch (buffer[0]) {
            case HELLO:
                cout << "Received HELLO" << endl;
                read_hello(buf_server_to_gui);
                buf_server_to_gui.reset_buffer();
                game.generate_lobby_respond(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case ACCEPTED_PLAYER:
                cout << "Received ACCEPTED_PLAYER" << endl;
                read_accepted_player(buf_server_to_gui);
                buf_server_to_gui.reset_buffer();
                game.generate_lobby_respond(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case GAME_STARTED:
                cout << "Received GAME_STARTED" << endl;
                read_game_started(buf_server_to_gui);
                is_game_started = true;
                break;
            case TURN:
                cout << "Received TURN" << endl;
                read_turn(buf_server_to_gui);
                buf_server_to_gui.reset_buffer();
                game.generate_game_respond(buf_server_to_gui);
                send_msg_to_gui();
                break;
            case GAME_ENDED:
                cout << "Received GAME_ENDED" << endl;
                read_game_ended(buf_server_to_gui);
                is_game_started = false;
                buf_server_to_gui.reset_buffer();
                game.reset_game();
                game.generate_lobby_respond(buf_server_to_gui);
                send_msg_to_gui();
                break;
            default:
                cout << "Received unknown message" << endl;
                // TODO handle unknown message
                break;
        }
//    }
    } while (msg_len != 0);
}

void Client::read_hello(Buffer &buf) {
    char buffer[2];
    read_str(tcp_socket_fd, buf);                                  // server name
    get_n_bytes_from_server(buffer, 1);
    buf.write_into_buffer(*(uint8_t *) buffer);   // players count
    get_n_bytes_from_server(buffer, 2);
    buf.write_into_buffer(*(uint16_t *) buffer);  // size x
    get_n_bytes_from_server(buffer, 2);
    buf.write_into_buffer(*(uint16_t *) buffer);  // size y
    get_n_bytes_from_server(buffer, 2);
    buf.write_into_buffer(*(uint16_t *) buffer);  // game length
    get_n_bytes_from_server(buffer, 2);
    buf.write_into_buffer(*(uint16_t *) buffer);  // explosion radius
    get_n_bytes_from_server(buffer, 2);
    buf.write_into_buffer(*(uint16_t *) buffer);  // bomb timer
    game = Game(buf);
    uint32_t map_len = 0;
    buf.write_into_buffer(map_len);
}

void Client::read_accepted_player(Buffer &buf) {
    read_player(tcp_socket_fd, buf);
    player_id_t player_id = buf_server_to_gui.read_1_byte();
    Player player(buf_server_to_gui);
    game.add_player(player_id, player);
}

void Client::read_game_started(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, 4);
    uint32_t map_len = be32toh(*(uint32_t *) buffer);
    cout << "Game started with " << map_len << " players" << endl;
    buf.print_buffer(buf.get_buffer(), buf.get_no_written_bytes());
    for (uint32_t i = 0; i < map_len; i++) {
        read_accepted_player(buf);
    }
}

void Client::read_turn(Buffer &buf) {
    char local_buf[sizeof(list_len_t)];
    get_n_bytes_from_server(local_buf, sizeof(turn_t));
    turn_t turn = be16toh(*(turn_t *) local_buf);
    game.set_turn(turn);
    get_n_bytes_from_server(local_buf, sizeof(list_len_t));
    list_len_t list_len = be32toh(*(list_len_t *) local_buf);
    cout << "List size: " << list_len << endl;
//    buf.print_buffer(buf.get_buffer(), buf.get_no_written_bytes());
    for (list_len_t i = 0; i < list_len; i++) {
        read_event(tcp_socket_fd, buf, game);
    }
}

void Client::read_game_ended(Buffer &buf) {
    char local_buf[sizeof(map_len_t)];
    get_n_bytes_from_server(local_buf, sizeof(map_len_t));
    map_len_t map_len = *(map_len_t *) local_buf;
    buf.write_into_buffer(map_len);
    for (map_len_t i = 0; i < map_len; i++) {
        get_n_bytes_from_server(local_buf, sizeof(player_id_t));
        player_id_t player_id = local_buf[0];
        get_n_bytes_from_server(local_buf, sizeof(score_t));
        score_t score = *(score_t *) local_buf;
        // TOOD dodac do mapy
    }
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
