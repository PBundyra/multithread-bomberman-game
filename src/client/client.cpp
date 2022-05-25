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
    socklen_t address_length = (socklen_t)
    sizeof(gui_addr);
    errno = 0;
    ssize_t received_length = recvfrom(udp_socket_fd, buf_server_to_gui.get_buffer(), BUFFER_SIZE,
                                       0,
                                       (struct sockaddr *) &gui_addr,
                                       &address_length);
    if (received_length < 0) {
        PRINT_ERRNO();
    }
    return (size_t) received_length;
}

void Client::send_msg_to_gui() {
    socklen_t
            address_length = (socklen_t)
    sizeof(gui_addr);
    ssize_t sent_length = sendto(udp_socket_fd, buf_server_to_gui.get_buffer(),
                                 buf_server_to_gui.get_no_written_bytes(), 0,
                                 (struct sockaddr *) &gui_addr, address_length);
    ENSURE(sent_length == (ssize_t) buf_server_to_gui.get_no_written_bytes());
}

void Client::parse_msg_from_gui(const size_t msg_len) {
    ENSURE(msg_len == 2 || msg_len == 1);
    cout << "Received message from GUI of length " << msg_len << endl;
    if (is_game_started) {  // sends PlaceBomb or PlaceBlock or Move
        uint8_t msg_code = buf_gui_to_server.read_1_byte();
        cout << "Sending " << (int) (msg_code + 1) << endl;
        buf_gui_to_server.overwrite_buffer((uint8_t)(msg_code + 1), (size_t) 0);
    } else {    // sends Join
        cout << "Sending Join" << endl;
        buf_gui_to_server.reset_buffer();
        buf_gui_to_server.write_into_buffer((uint8_t) JOIN);
        buf_gui_to_server.write_into_buffer((uint8_t) player_name.size());
        buf_gui_to_server.write_into_buffer(player_name.c_str(), player_name.size());
//        buf_gui_to_server.print_buffer();
    }
}

size_t Client::get_n_bytes_from_server(void *buffer, const size_t n) {
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

void Client::send_msg_to_server() {
    cout << "Sending message to server of length " << buf_gui_to_server.get_no_written_bytes() << endl;
    errno = 0;
    ssize_t sent_length = send(tcp_socket_fd, buf_gui_to_server.get_buffer(),
                               buf_gui_to_server.get_no_written_bytes(), 0);
    if (sent_length < 0) {
        PRINT_ERRNO();
    }
    ENSURE(sent_length == (ssize_t) buf_gui_to_server.get_no_written_bytes());
    cout << "Sent message to server" << endl;
}

bool Client::parse_msg_from_server(const char *msg, const size_t msg_len) {
    parse_hello(msg, msg_len);
    return true;
}

void Client::gui_to_server_handler() {
    size_t msg_len;
    do {
        buf_gui_to_server.reset_buffer();
        msg_len = get_msg_from_gui();
        parse_msg_from_gui(msg_len);
        send_msg_to_server();
    } while (msg_len != 0);
}

void Client::server_to_gui_handler() {
    char buffer[1];
    size_t msg_len;
    do {
        buf_server_to_gui.reset_buffer();
        msg_len = get_n_bytes_from_server(buffer, 1);
        switch (buffer[0]) {
            case HELLO:
                cout << "Received HELLO" << endl;
                read_hello(buf_server_to_gui);
                break;
            case ACCEPTED_PLAYER:
                cout << "Received ACCEPTED_PLAYER" << endl;
                read_accepted_player(buf_server_to_gui);
                break;
            case GAME_STARTED:
                cout << "Received GAME_STARTED" << endl;
                read_game_started(buf_server_to_gui);
                is_game_started = true;
                break;
            case TURN:
                cout << "Received TURN" << endl;
                read_turn(buf_server_to_gui);
                break;
            case GAME_ENDED:
                cout << "Received GAME_ENDED" << endl;
                read_game_ended(buf_server_to_gui);
                is_game_started = false;
                break;
            default:
                cout << "Received unknown message" << endl;
                // TODO handle unknown message
                break;
        }
    } while (msg_len != 0);
}

void Client::read_str(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, 1);
    uint8_t str_len = (uint_8_t) buffer[0];
    buf.write_into_buffer(str_len);
    get_n_bytes_from_server(buffer, str_len);
    buf.write_into_buffer(buffer, str_len);
}

void Client::read_hello(Buffer &buf) {
    char buffer[2];
    read_str(buf);                                  // server name
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
}

void Client::parse_hello(const char *msg, const size_t msg_len) {
    size_t no_read_bytes = 0;
//    ENSURE((uint8_t) msg[no_read_bytes] == 0);
//    buf_server_to_gui.write_into_buffer(*(uint8_t * )(msg + no_read_bytes));
//    no_read_bytes++;
    uint8_t server_name_len = *(uint8_t * )(msg + no_read_bytes);
    buf_server_to_gui.write_into_buffer(server_name_len);
    no_read_bytes++;
    // server name
    buf_server_to_gui.write_into_buffer(msg + no_read_bytes, server_name_len);
    no_read_bytes += server_name_len;
    // players count
    buf_server_to_gui.write_into_buffer(*(uint8_t * )(msg + no_read_bytes));
    no_read_bytes += 1;
    // size x
    buf_server_to_gui.write_into_buffer(*(uint16_t * )(msg + no_read_bytes));
    no_read_bytes += 2;
    // size y
    buf_server_to_gui.write_into_buffer(*(uint16_t * )(msg + no_read_bytes));
    no_read_bytes += 2;
    // game length
    buf_server_to_gui.write_into_buffer(*(uint16_t * )(msg + no_read_bytes));
    no_read_bytes += 2;
    // explosion radius
    buf_server_to_gui.write_into_buffer(*(uint16_t * )(msg + no_read_bytes));
    no_read_bytes += 2;
    // bomb timer
    buf_server_to_gui.write_into_buffer(*(uint16_t * )(msg + no_read_bytes));

//    buf_server_to_gui.print_buffer(buf_server_to_gui.get_buffer(), buf_server_to_gui.get_no_written_bytes());
    game = Game(buf_server_to_gui);
}

void Client::read_accepted_player(Buffer &buf) {
    read_player(buf);
    player_id_t player_id = buf_server_to_gui.read_1_byte();
    Player player(buf_server_to_gui);
    game.add_player(player_id, player);
}

void Client::read_game_started(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, 4);
    uint32_t map_size = *(uint32_t *) buffer;
    buf.write_into_buffer(map_size);
    for (uint32_t i = 0; i < map_size; i++) {
        read_player(buf);
    }
    parse_game_started();
}

void Client::parse_game_started() {}

void Client::read_turn(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, 4);
    uint32_t list_size = *(uint32_t *) buffer;
    buf.write_into_buffer(list_size);
    for (uint32_t i = 0; i < list_size; i++) {
        read_event(buf);
    }
    parse_turn();
}

void Client::parse_turn(const char *msg, const size_t msg_len) {}

void Client::read_game_ended(Buffer &buf) {
    char buffer[BUFFER_SIZE];
    get_n_bytes_from_server(buffer, 4);
    uint32_t map_size = *(uint32_t *) buffer;
    buf.write_into_buffer(map_size);
    for (uint32_t i = 0; i < map_size; i++) {
        get_n_bytes_from_server(buffer, sizeof(player_id_t));
        player_id_t player_id = buffer[0];
        get_n_bytes_from_server(buffer, sizeof(score_t));
        score_t score = *(score_t *) buffer;
        // TOOD dodac do mapy
    }
    parse_game_started();
}

void Client::parse_game_ended(const char *msg, const size_t msg_len) {}


void Client::run() {
    thread
    gui_to_server_thread(bind(&Client::gui_to_server_handler, this));
    thread
    server_to_gui_thread(bind(&Client::server_to_gui_handler, this));
    gui_to_server_thread.join();
    server_to_gui_thread.join();
}



//        while (true) {
// niezaleznie od stanu jesli dostajemy komunikat od serwera go obłusgujemy
// jesli komunikat to AcceptedPlayer/Turn to wysylami zmiany do GUI
// jesli dostales komunikat GameStarted to trzeba zmienic stan na Game
// jesli dostales komunikat GameEnded to trzeba zmienic stan na Lobby

//            if (state == GameState::Lobby) {
// wszystkie komunikaty od gui leca do serwera jako Join
//            } else {
// wszystkie komunikaty od gui leca tak jak przyszly
//            }

int main(int argc, char **argv) {
    input_params_t input_params = parse_cli_params(argc, argv);
    Client client = Client(input_params);
    client.run();
    return 0;
}
