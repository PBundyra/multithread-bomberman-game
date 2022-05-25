#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string.h>
#include <limits>
#include <iostream>
#include <sstream>
#include <bitset>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <atomic>


#include "buffer.h"
#include "events.h"
#include "lobby.h"
#include "map.h"
#include "player.h"
#include "utils.h"
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
using port_t = uint16_t;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::ip::address;

typedef struct input_params_t {
    string player_name;
    port_t port;
    port_t gui_port;
    port_t server_port;
    string gui_host;
    string server_host;
    address tcp_addr;
    address udp_addr;
} input_params_t;

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
        size_t tcp_port_ind = server_address.find_last_of(':');
        params.server_port = atoi(server_address.substr(tcp_port_ind + 1).c_str());
        params.server_host = server_address.substr(0, tcp_port_ind);
        params.tcp_addr = address::from_string(params.server_host);
        size_t udp_port_ind = display_address.find_last_of(':');
        params.gui_port = atoi(display_address.substr(udp_port_ind + 1).c_str());
        params.gui_host = display_address.substr(0, udp_port_ind);
        params.udp_addr = address::from_string(params.gui_host);
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

void print_cli(input_params_t &params) {
//    printf("display-address: %s\n", params.display_address.c_str());
    printf("player-name: %s\n", params.player_name.c_str());
    printf("port: %d\n", params.port);
//    printf("server-address: %s\n", params.server_address.c_str());
}


// Buffer.h
//void insert(Player player);
// variant [Hello, AcceptedGame, GameStarted, Turn, GameEnded]


// Klasy
// Server, Buffer, Map, ServerMessage (abstrakcyjna), ActionMessage, Klasy dotyczace Parsera - kazdy stan to osobna klasa

// Turn

// vector<unique_ptr<Event>>;

//Map.h
void change_map(unique_ptr<BombPlaced> event);

void change_map(unique_ptr<BombExploded> event);

//for po vectorze
//    map.change_map(event);



// W petli wykonujemy
// Dostajesz wiadomosc od servera (dluga) - na poczatku stan całej mapy, a następnie zmiany - dynamicznie tworzymy odpowiednie struktury
// Po parsowaniu dostajemy wariant
// Na podstawie tej unii dokonuje zmian w classie Map/Game
// Wysyla wiadomosc do GUI (dluga) ze stanem mapy
// Dostajesz wiadomosc od GUI (krotka) - input gracza - z klawiatury
// Zapisujemy do buffera
// Wysyła wiadomosc do servera (krotka)  - input gracza - z klawiatury

// Klient:
// - laczy sie z GUI i Serverem
// - tworzy 2 buffory, mape
// - odpala 2 watki
// - pierwszy watek przesyla wiadomosci z GUI do Servera
// - drugi watek przesyla wiadomosci z Servera do GUI


class Client {
private:
//    unique_ptr<Server> server;
    Buffer buf;
    Map map;
//    Lobby lobby;
    atomic_bool is_game_started;
    string name;


    using list_len_t = uint32_t;
    using map_len_t = uint32_t;


    void parse_events(vector<shared_ptr<Event>> events) {
        // w petli
        // odczytaj z buffera
        // stworz kolejny event
        // spushuj zmiany
    };

    void parse_hello(const char *msg, size_t len) {
        size_t no_read_el = 0;
        assert((uint8_t) msg[no_read_el] == 0);
        no_read_el++;
        unsigned int name_len = *(uint8_t *) (msg + no_read_el);
        no_read_el++;
        cout << "name len: " << name_len << "\n";
        string server_name = "";
        unsigned i = no_read_el;
        for (; i < no_read_el + name_len; ++i) {
            server_name += msg[i];
        }
        no_read_el += name_len;
        cout << "server name: " << server_name << "\n";
        uint8_t no_players = *(uint8_t *) (msg + no_read_el);

        cout << "no players: " << static_cast<int>(msg[no_read_el]) << "\n";
        no_read_el += 1;
        uint16_t size_x = *(uint16_t *) (msg + no_read_el);
        cout << "size_x: " << be16toh(*(uint16_t *) (msg + no_read_el)) << "\n";
        no_read_el += 2;
        uint16_t size_y = *(uint16_t *) (msg + no_read_el);
        cout << "size_y: " << be16toh(*(uint16_t *) (msg + no_read_el)) << "\n";
        no_read_el += 2;
        uint16_t game_length = *(uint16_t *) (msg + no_read_el);
        cout << "game len: " << be16toh(*(uint16_t *) (msg + no_read_el)) << "\n";
        no_read_el += 2;
        cout << "explosion radius: " << be16toh(*(uint16_t *) (msg + no_read_el)) << "\n";
        no_read_el += 2;
        cout << "bomb timer: " << be16toh(*(uint16_t *) (msg + no_read_el)) << "\n";
        this->map = Map(server_name, size_x, size_y, game_length);
    }

    void connect_to_gui(const input_params_t &params);

    void connect_to_server(const input_params_t &params);

public:
    Client(input_params_t &input_params) : name(input_params.player_name) {
        is_game_started = false;
    }



    void run(input_params_t &params) {
        try {
            boost::asio::io_context io_context;
            tcp_server server1(io_context, params.tcp_addr, params.server_port);
            udp_server server2(io_context, params.udp_addr, params.gui_port);
            io_context.run();
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
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



        // odczytaj komunikat od servera
        // jeśli dostałeś GameStarted to spectate bo gra się zaczęła
        // jesli dostałeś AcceptedPlayer to znaczy że gra się nie zaczęła, można jeszcze dołączyć
        // jeśli rozgrywka jest w Lobby to jeśli dostałeś komunikat od GUI, to wyślij Join do Servera
        // po zaakceptowaniu wejdz w tryb play
        // odczytujemy wiadomosc od servera
    };
};

void Client::gui_to_server_handler() {
    for (int i = 0; i < 10; i++)
        cout << "gui_to_server_handler\n";
}

void Client::server_to_gui_handler() {
    for (int i = 0; i < 10; i++)
        cout << "server_to_gui_handler\n";
}

void Client::receive_hello() {

}

int main(int argc, char **argv) {
    input_params_t input_params = parse_cli_params(argc, argv);
    Client client = Client(input_params);
    client.run(input_params);
    return 0;
}
