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

struct sockaddr_in get_send_address(const char *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, nullptr, &hints, &address_result));

    struct sockaddr_in send_address;
    send_address.sin_family = AF_INET; // IPv4
    send_address.sin_addr.s_addr =
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
    send_address.sin_port = htons(port); // port from the command line

    freeaddrinfo(address_result);

    return send_address;
}


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

// W petli wykonujemy
// Dostajesz wiadomosc od servera (dluga) - na poczatku stan całej mapy, a następnie zmiany - dynamicznie tworzymy odpowiednie struktury
// Po parsowaniu dostajemy wariant
// Na podstawie tej unii dokonuje zmian w classie Map/Game
// Wysyla wiadomosc do GUI (dluga) ze stanem mapy
// Dostajesz wiadomosc od GUI (krotka) - input gracza - z klawiatury
// Zapisujemy do buffera
// Wysyła wiadomosc do servera (krotka)  - input gracza - z klawiatury
void Client::gui_to_server_handler() {
    for (int i = 0; i < 10; i++) {
        if (is_game_started) {
            cout << "Game started" << endl;
        } else {
            cout << "Game not started" << endl;
        }
        sleep(1);
    }
}

void Client::server_to_gui_handler() {
    receive_hello();
}

void Client::parse_events(vector<shared_ptr<Event>> &events) {
    // w petli
    // odczytaj z buffera
    // stworz kolejny event
    // spushuj zmiany
};

void Client::parse_hello(const char *msg) {
    size_t no_read_bytes = 0;
    assert((uint8_t) msg[no_read_bytes++] == 0);
    uint8_t server_name_len = *(uint8_t *) (msg + no_read_bytes);
    buf_server_to_gui.write_into_buffer(server_name_len);
    no_read_bytes++;
    // server name
    buf_server_to_gui.write_str_into_buffer(msg + no_read_bytes, server_name_len);
    no_read_bytes += server_name_len;
    // players count
    buf_server_to_gui.write_into_buffer(*(uint8_t *) (msg + no_read_bytes));
    no_read_bytes += 1;
    // size x
    buf_server_to_gui.write_into_buffer(be16toh(*(uint16_t *) (msg + no_read_bytes)));
    no_read_bytes += 2;
    // size y
    buf_server_to_gui.write_into_buffer(be16toh(*(uint16_t *) (msg + no_read_bytes)));
    no_read_bytes += 2;
    // game length
    buf_server_to_gui.write_into_buffer(be16toh(*(uint16_t *) (msg + no_read_bytes)));
    no_read_bytes += 2;
    // explosion radius
    buf_server_to_gui.write_into_buffer(be16toh(*(uint16_t *) (msg + no_read_bytes)));
    no_read_bytes += 2;
    // bomb timer
    buf_server_to_gui.write_into_buffer(be16toh(*(uint16_t *) (msg + no_read_bytes)));
    map = Map(buf_server_to_gui);
}

void Client::receive_hello() {
    char msg[1024];
    size_t received_len = receive_message(tcp_socket_fd, msg, MAX_BUFFER_SIZE - 1, 0);
    if (received_len == 0) {
        cout << "Server closed connection\n";
        exit(0);
    }
    parse_hello(msg);
}

void Client::run(input_params_t &params) {
    thread gui_to_server_thread(bind(&Client::gui_to_server_handler, this));
    thread server_to_gui_thread(bind(&Client::server_to_gui_handler, this));
    gui_to_server_thread.join();
    server_to_gui_thread.join();
}

// Klient:
// - laczy sie z GUI i Serverem
// - tworzy 2 buffory, mape
// - odpala 2 watki
// - pierwszy watek przesyla wiadomosci z GUI do Servera
// - drugi watek przesyla wiadomosci z Servera do GUI


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
    client.run(input_params);
    return 0;
}
