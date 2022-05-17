#include "stdio.h"
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <string.h>

#include "buffer.h"
#include "events.h"
#include "lobby.h"
#include "map.h"
#include "player.h"
#include "utils.h"


namespace po = boost::program_options;
namespace asio = boost::asio;
using namespace std;
namespace tcp = asio::ip::tcp;
namespace udp = asio::ip::udp;

typedef struct input_params_t {
    string display_address;
    string player_name;
    uint16_t port;
    string server_address;
} input_params_t;

input_params_t parse_cli_params(int argc, char **argv) {
    input_params_t params;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("display-address,d", po::value<string>(), "display address")
        ("player-name,n", po::value<string>(), "player name")
        ("port,p", po::value<uint16_t>(), "port")
        ("server-address,s", po::value<string>(), "server address");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        exit(0);
    }
    if (vm.count("display-address")) {
        params.display_address = vm["display-address"].as<string>();
    } else {
        params.display_address = "None";
    }
    if (vm.count("player-name")) {
        params.player_name = vm["player-name"].as<string>();
    } else {
    params.player_name = "None";
    }
    if (vm.count("port")) {
        params.port = vm["port"].as<uint16_t>();
    } else {
        params.port = 0;
    }
    if (vm.count("server-address")) {
        params.server_address = vm["server-address"].as<string>();
    } else {
        params.server_address = "None";
    }
    return params;
}

void print_cli(input_params_t &params) {
    printf("display-address: %s\n", params.display_address.c_str());
    printf("player-name: %s\n", params.player_name.c_str());
    printf("port: %d\n", params.port);
    printf("server-address: %s\n", params.server_address.c_str());
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

for po vectorze
    map.change_map(event);



// W petli wykonujemy
// Dostajesz wiadomosc od servera (dluga) - na poczatku stan całej mapy, a następnie zmiany - dynamicznie tworzymy odpowiednie struktury
// Po parsowaniu dostajemy wariant
// Na podstawie tej unii dokonuje zmian w classie Map/Game
// Wysyla wiadomosc do GUI (dluga) ze stanem mapy
// Dostajesz wiadomosc od GUI (krotka) - input gracza - z klawiatury
// Zapisujemy do buffera
// Wysyła wiadomosc do servera (krotka)  - input gracza - z klawiatury
class Gui{
public:
    void send_message(Buffer &buffer);
};

enum GameState {
    Lobby,
    Game,
};

class Client {
private:
//    unique_ptr<Server> server;
    Buffer buf;
    Gui gui;
    Map map;
    Lobby lobby;
    GameState state;
    string name;


    using list_len_t = uint32_t;
    using map_len_t = uint32_t;


    void parse_events(vector<shared_ptr<Event>> events){
        // w petli
        // odczytaj z buffera
        // stworz kolejny event
        // spushuj zmiany
    };

    void spectate(){
        // dodaje wszystkich graczy uzywajac komunikatu GameStarted
        vector<shared_ptr<Event>> events;
        parse_events(events);
        map.apply_changes(events);
        map.generate_respond(buf);
        gui.send_message(buf);
    };

    void play(){
        // używając poll
        // otrzymuj wiadomosc od servera
        // jak tylko ją otrzymujesz parsuj zmiany na mapie
        // wysyłaj do gui mape
        // w międzyczasie otrzymuj wiadomosc od gui
        // napierdalaj do servera wszystko co przychodzi od gui
    };

public:
    Client(input_params_t &input_params){
        name = input_params.player_name;
        state = Lobby;
    }

    void connect_to_gui(const input_params_t &input_params){
        asio::io_context io_context;
        udp::resolver resolver(io_context);
        udp::endpoint endpoint = *resolver.resolve({input_params.display_address, to_string(input_params.port)});

        // wyciagnij z input_params IP i port
        // uzywajac asio podlacz sie do gui po UDP
    };

    void connect_to_server(const input_params_t &input_params){
        asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        asio::connect(socket, resolver.resolve({input_params.server_address, to_string(input_params.port)}));

        // wyciagnij z input_params IP i port
        // uzywajac asio podlacz sie do servera po TCP
        // odczytaj komunikat Hello
        // stwórz mapę
    };

    void run(){
        while (true){
            // niezaleznie od stanu jesli dostajemy komunikat od serwera go obłusgujemy
            // jesli komunikat to AcceptedPlayer/Turn to wysylami zmiany do GUI
            // jesli dostales komunikat GameStarted to trzeba zmienic stan na Game
            // jesli dostales komunikat GameEnded to trzeba zmienic stan na Lobby

            if (state == GameState::Lobby){
                // wszystkie komunikaty od gui leca do serwera jako Join
            }
            else {
                // wszystkie komunikaty od gui leca tak jak przyszly
            }



            // uzywajac polla
            // odczytaj komunikat od servera
            // jeśli dostałeś GameStarted to spectate bo gra się zaczęła
            // jesli dostałeś AcceptedPlayer to znaczy że gra się nie zaczęła, można jeszcze dołączyć
            // jeśli rozgrywka jest w Lobby to jeśli dostałeś komunikat od GUI, to wyślij Join do Servera
            // po zaakceptowaniu wejdz w tryb play
            // odczytujemy wiadomosc od servera
        }
    };
};

int main(int argc, char **argv) {
    input_params_t input_params = parse_cli_params(argc, argv);
    print_cli(input_params);
    asio::ip::address adr = asio::ip::address::make_address(input_params.server_address);
//    Client client = Client();
//    client.connect_to_server(input_params);
//    client.connect_to_gui(input_params);
//    client.run();
    return 0;
}
// variant [Hello, AcceptedGame, GameStarted, Turn, GameEnded]


// Klasy
// Server, Buffer, Map, ServerMessage (abstrakcyjna), ActionMessage, Klasy dotyczace Parsera - kazdy stan to osobna klasa

// W petli wykonujemy
// Dostajesz wiadomosc od servera (dluga) - na poczatku stan całej mapy, a nastepnie zmiany - dynamicznie tworzymy odpowiednie struktury
// Po parsowaniu dostajemy wariant
// Na podstawie tej unii dokonuje zmian w classie Map/Game
// Wysyla wiadomosc do GUI (dluga) ze stanem mapy
// Dostajesz wiadomosc od GUI (krotka) - input gracza - z klawiatury
// Zapisujemy do buffera
// Wysyła wiadomosc do servera (krotka)  - input gracza - z klawiatury
