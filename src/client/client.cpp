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


//#include "buffer.h"
#include "events.h"
#include "lobby.h"
#include "map.h"
#include "player.h"
#include "utils.h"


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
    port_t udp_port;
    port_t tcp_port;
    string udp_address;
    string tcp_address;
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
        params.tcp_port = atoi(server_address.substr(tcp_port_ind + 1).c_str());
        params.tcp_address = server_address.substr(0, tcp_port_ind);
        params.tcp_addr = address::from_string(params.tcp_address);
        size_t udp_port_ind = display_address.find_last_of(':');
        params.udp_port = atoi(display_address.substr(udp_port_ind + 1).c_str());
        params.udp_address = display_address.substr(0, udp_port_ind);
        params.udp_addr = address::from_string(params.udp_address);
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

enum GameState {
    Lobby,
    Game,
};

//
//class Buffer {
//private:
//    friend class Client;
//
//    char buffer[MAX_DATAGRAM_SIZE]{};
//    size_t no_elements;
//
//    size_t read_msg(int socket_fd, struct sockaddr_in *client_address) {
//        auto address_length = (socklen_t)
//        sizeof(*client_address);
//        int flags = 0; // we do not request anything special
//        errno = 0;
//        ssize_t len = recvfrom(socket_fd, buffer, MAX_DATAGRAM_SIZE, flags,
//                               (struct sockaddr *) client_address, &address_length);
//        if (len < 0) {
//            PRINT_ERRNO();
//        }
//        no_elements = len;
//        return (size_t) len;
//    }
//
//    [[nodiscard]] uint16_t get_msg_size() const { return uint16_t(no_elements); }
//
//    msg_id_t get_msg_id() { return (msg_id_t) buffer[0]; }
//
//    event_id_t get_event_id() {
//        assert(get_msg_id() == MsgID::GET_RESERVATION_MSG_ID);
//        return ntohl(*(uint32_t * )(buffer + sizeof(uint8_t)));
//    }
//
//    ticket_cnt_t get_ticket_cnt() {
//        assert(get_msg_id() == MsgID::GET_RESERVATION_MSG_ID);
//        return ntohs(*(uint16_t * )(buffer + sizeof(uint8_t) + sizeof(uint32_t)));
//    }
//
//    reservation_id_t get_reservation_id() {
//        assert(get_msg_id() == MsgID::GET_TICKET_MSG_ID);
//        return ntohl(*(uint32_t * )(buffer + sizeof(uint8_t)));
//    }
//
//    string get_cookie() {
//        assert(get_msg_id() == MsgID::GET_TICKET_MSG_ID);
//        return {buffer + sizeof(uint8_t) + sizeof(uint32_t), COOKIE_LEN};
//    }
//
//public:
//    Buffer() : no_elements(0) {}
//
//    void write_into_buffer(uint8_t msg) {
//        auto net_val = msg;
//        memcpy(buffer + no_elements, &net_val, sizeof(uint8_t));
//        no_elements += sizeof(uint8_t);
//    }
//
//    void write_into_buffer(uint16_t msg) {
//        auto net_val = htobe16(msg);
//        memcpy(buffer + no_elements, &net_val, sizeof(uint16_t));
//        no_elements += sizeof(uint16_t);
//    }
//
//    void write_into_buffer(uint32_t msg) {
//        auto net_val = htobe32(msg);
//        memcpy(buffer + no_elements, &net_val, sizeof(uint32_t));
//        no_elements += sizeof(uint32_t);
//    }
//
//    void write_into_buffer(uint64_t msg) {
//        auto net_val = htobe64(msg);
//        memcpy(buffer + no_elements, &net_val, sizeof(uint64_t));
//        no_elements += sizeof(uint64_t);
//    }
//
//    void write_str_into_buffer(string &msg) {
//        memcpy(buffer + no_elements, msg.c_str(), msg.size());
//        no_elements += msg.size();
//    }
//
//    [[nodiscard]] size_t get_no_elements() const { return no_elements; }
//
//    string cpy_buffer() { return {buffer, no_elements}; }
//};
static std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context &io_context) {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket &socket() {
        return socket_;
    }

    void start() {
        message_ = make_daytime_string();


        boost::asio::async_write(socket_, boost::asio::buffer(message_),
                                 boost::bind(&tcp_connection::handle_write, shared_from_this()));
        std::cout << "Wyslalem po tcp\n";
    }

private:
    tcp::socket socket_;
    std::string message_;

    friend class Client;

    tcp_connection(boost::asio::io_context &io_context)
            : socket_(io_context) {
    }

    void handle_write() {
    }

};

class tcp_server {
public:
    tcp_server(boost::asio::io_context &io_context, const address &addr, port_t port)
            : tcp_socket(io_context, tcp::endpoint(addr, port)) {
        start_accept();
    }

private:
    friend class Client;

    tcp::socket tcp_socket;
    char tcp_buf[1024];
//    tcp::endpoint tcp_endpoint;
//    boost::asio::io_context &io_context;
//    tcp::acceptor acceptor;

    void start_accept() {
//        tcp_connection::pointer new_connection = tcp_connection::create(io_context);
//
        tcp_socket.async_read_some(boost::asio::buffer(tcp_buf),
                                   boost::bind(&tcp_server::handle_accept, this,
                                               boost::asio::placeholders::error));
//        acceptor.async_accept(new_connection->tcp_socket(),
//                              boost::bind(&tcp_server::handle_accept, this, new_connection,
//                                          boost::asio::placeholders::error));
    }

    void handle_accept(const tcp_connection::pointer &new_connection, const boost::system::error_code &error) {
//        if (!error) {
//            new_connection->start();
//        }
        tcp_socket.async_write_some(boost::asio::buffer(tcp_buf),
                                    boost::bind(&tcp_server::handle_accept, this,
                                                boost::asio::placeholders::error));
        start_accept();
    }
};

class udp_server {
public:
    udp_server(boost::asio::io_context &io_context, const address& addr, port_t port)
            : udp_socket(io_context, udp::endpoint(addr, port)), remote_endpoint(addr, port) {
        start_receive();
    }

private:
    friend class Client;

    udp::socket udp_socket;
    udp::endpoint remote_endpoint;
//    boost::array<char, 1> recv_buffer;
    char udp_buf[1024];

    void start_receive() {
//        tcp_socket.async_read_some(boost::asio::buffer(recv_buffer),
//                               boost::bind(&udp_server::handle_receive, this,
//                                           boost::asio::placeholders::error,
//                                           boost::asio::placeholders::bytes_transferred));
        udp_socket.async_receive_from(
                boost::asio::buffer(udp_buf), remote_endpoint,
                boost::bind(&udp_server::handle_receive, this,
                            boost::asio::placeholders::error));
    }
//    void udp_handler(const boost::system::error_code &error, size_t bytes_transferred) {
//        cout << "ESSA UDALO SIE\n";
//    }

    void handle_receive(const boost::system::error_code &error, size_t bytes_transferred) {
        if (!error) {
            boost::shared_ptr<std::string> message(
                    new std::string(make_daytime_string()));

            udp_socket.async_send_to(boost::asio::buffer(*message), remote_endpoint,
                                     boost::bind(&udp_server::handle_send, this, message));
            std::cout << "Wyslalem po udp\n";

            start_receive();
        }
    }


    void handle_send(boost::shared_ptr<std::string> /*message*/) {
    }

};


class Client {
private:
//    unique_ptr<Server> server;
    Buffer buf;
    Map map;
//    Lobby lobby;
    GameState state;
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

public:
    Client(input_params_t &input_params) {
        name = input_params.player_name;
        state = Lobby;
    }

    void connect_to_gui(const input_params_t &params) {
        asio::io_context io_context;
        ip::udp::resolver resolver(io_context);
//        size_t port_ind = input_params.display_address.find_last_of(':');
//        uint16_t port = atoi(input_params.display_address.substr(port_ind + 1).c_str());
        ip::udp::endpoint endpoint = *resolver.resolve({params.udp_address, to_string(params.udp_port)});

        // wyciagnij z input_params IP i port
        // uzywajac asio podlacz sie do gui po UDP
    };

    void connect_to_server(const input_params_t &params) {
        asio::io_context io_context;
        ip::tcp::socket socket(io_context);
        ip::tcp::resolver resolver(io_context);
        asio::connect(socket, resolver.resolve(params.tcp_address, to_string(params.tcp_port)));

        char buf[1024];
        boost::system::error_code error;
        size_t len = socket.read_some(boost::asio::buffer(buf, 1024), error);

        if (error) {
            throw boost::system::system_error(error); // Some other error.
        }
        parse_hello(buf, len);

        for (size_t i = 0; i < len; ++i) {
            cout << (int) buf[i] << " ";
            bitset<8> bs4(buf[i]);
            cout << bs4 << " ";

            if (i % 6 == 0 && i != 0)
                cout << endl;
        }
        cout << endl;
        // wyciagnij z input_params IP i port
        // uzywajac asio podlacz sie do servera po TCP
        // odczytaj komunikat Hello
        // stwórz mapę
    };

    void run(input_params_t &params) {
        try {
            boost::asio::io_context io_context;
            tcp_server server1(io_context, params.tcp_addr, params.tcp_port);
            udp_server server2(io_context, params.udp_addr, params.udp_port);
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



            // uzywajac polla
            // odczytaj komunikat od servera
            // jeśli dostałeś GameStarted to spectate bo gra się zaczęła
            // jesli dostałeś AcceptedPlayer to znaczy że gra się nie zaczęła, można jeszcze dołączyć
            // jeśli rozgrywka jest w Lobby to jeśli dostałeś komunikat od GUI, to wyślij Join do Servera
            // po zaakceptowaniu wejdz w tryb play
            // odczytujemy wiadomosc od servera
//        }
    };
};

int main(int argc, char **argv) {
    input_params_t input_params = parse_cli_params(argc, argv);
    print_cli(input_params);
//    cout << asio::ip::address::from_string(input_params.server_address).to_string() << endl;
//    asio::ip::address adr = asio::ip::address::from_string(input_params.server_address);
    Client client = Client(input_params);
//    client.connect_to_server(input_params);
//    client.connect_to_gui(input_params);
    client.run(input_params);
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
