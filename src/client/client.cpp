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

#define MAX_BUFFER_SIZE 1024
#define JOIN 0
#define HELLO 0
#define ACCEPTED_PLAYER 1
#define GAME_STARTED 2
#define TURN 3
#define GAME_ENDED 4
#define BOMB_PLACED 0
#define BOMB_EXPLODED 1
#define PLAYER_MOVED 2
#define BLOCK_PLACEd 3

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
//    receive_hello();
    char buffer[BUFFER_SIZE];
    size_t msg_len;
    do {
        buf_server_to_gui.reset_buffer();
        msg_len = get_msg_from_server(buffer, 1);
        ENSURE(msg_len == 1);
        switch (buffer[0]) {
            case HELLO:
                read_hello();
                cout << "Received HELLO" << endl;
                break;
            case ACCEPTED_PLAYER:
                read_accepted_player();
                cout << "Received ACCEPTED_PLAYER" << endl;
                break;
            case GAME_STARTED:
                cout << "Received GAME_STARTED" << endl;
                is_game_started = true;
                break;
            case TURN:
                cout << "Received TURN" << endl;
                break;
            case GAME_ENDED:
                cout << "Received GAME_ENDED" << endl;
                is_game_started = false;
                break;
            default:
                cout << "Received unknown message" << endl;
                // TODO handle unknown message
                break;
        }
    } while (msg_len != 0);
}

size_t Client::read_str(const char *msg) {
    get_n_bytes_from_server(msg, 1);
    get_n_bytes_from_server(msg + 1, msg[0]);
    return msg[0] + 1;
}

void Client::read_hello() {
    char buffer[BUFFER_SIZE];
    size_t msg_len = read_str(buffer);
    msg_len += get_n_bytes_from_server(buffer + msg_len, 11);
    parse_hello(buffer, msg_len);
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
    map = Map(buf_server_to_gui);
}

void Client::read_accepted_player() {
    char buffer[BUFFER_SIZE];
    size_t msg_len = read_str(buffer);
    msg_len += read_str(buffer + msg_len);
    parse_accepted_player(buffer, msg_len);
}

void parse_accepted_player(const char *msg, const size_t msg_len){}

void read_game_started(){}

void parse_game_started(const char *msg, const size_t msg_len){}

void read_turn(){}

void parse_turn(const char *msg, const size_t msg_len){}

void read_game_ended(){}

void parse_game_ended(const char *msg, const size_t msg_len){}


void Client::receive_hello() {
    char msg[BUFFER_SIZE];
    size_t received_len = receive_message_tcp(tcp_socket_fd, msg, MAX_BUFFER_SIZE - 1, 0);
    if (received_len == 0) {
        cout << "Server closed connection\n";
        exit(0);
    }
    parse_hello(msg, received_len);
    cout << "Parsed hello\n";
}

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
