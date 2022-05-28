#ifndef CLIENT_H
#define CLIENT_H

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <cstring>
#include <limits>
#include <iostream>
#include <sstream>
#include <bitset>
#include <arpa/inet.h>
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
#include "game.h"
#include "player.h"
#include "utils.h"
#include "err.h"
#include "common.h"

typedef struct input_params_t {
    port_t port;
    std::string player_name;
    port_t gui_port;
    std::string gui_host;
    port_t server_port;
    char *server_host;
    struct sockaddr_in6 server_addr;
    struct sockaddr_in6 gui_addr;
} input_params_t;

input_params_t parse_cli_params(int argc, char **argv);


inline static int connect_with_server(char *host, port_t port) {
    INFO("Getting address for host " << host << " and port " << port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_DGRAM;
//    hints.ai_family = PF_INET6;
//    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *addr_res;
    CHECK(getaddrinfo(host, nullptr, &hints, &addr_res));

    int socket_fd = socket(addr_res->ai_family, addr_res->ai_socktype, 0);
    ENSURE(socket_fd >= 0);
    INFO("TCP socket created");
    CHECK_ERRNO(connect(socket_fd, addr_res->ai_addr, addr_res->ai_addrlen));

    freeaddrinfo(addr_res);
    INFO("Connected to server");
    return socket_fd;
}

class Client {
private:
    port_t port;
    std::string player_name;
    port_t gui_port;
    std::string gui_host;
    port_t server_port;
    char *server_host;
    struct sockaddr_in6 server_addr;
    struct sockaddr_in6 gui_addr;

    Buffer buf_server_to_gui;
    Buffer buf_gui_to_server;
    Game game;
    std::atomic_bool is_game_started;
    int server_socket_fd;
    int gui_socket_fd;

    void read_hello(Buffer &buf);

    void read_accepted_player(Buffer &buf);

    void read_game_started(Buffer &buf);

    void read_turn(Buffer &buf);

    void read_game_ended(Buffer &buf);

    size_t get_msg_from_gui(Buffer &buf);

    void send_msg_to_gui(Buffer &buf);

    void parse_msg_from_gui(size_t msg_len);

    size_t get_n_bytes_from_server(void *buffer, size_t n) const;

    void send_msg_to_server(Buffer &buf);

    [[noreturn]] void gui_to_server_handler();

    [[noreturn]] void server_to_gui_handler();

public:
    explicit Client(input_params_t &input_params) : port(input_params.port), player_name(input_params.player_name),
                                                    gui_port(input_params.gui_port), gui_host(input_params.gui_host),
                                                    server_port(input_params.server_port),
                                                    server_host(input_params.server_host),
                                                    server_addr(input_params.server_addr),
                                                    gui_addr(input_params.gui_addr),
                                                    is_game_started(false) {
        server_socket_fd = connect_with_server(server_host, server_port);
//        server_socket_fd = open_tcp_ip6_socket();
//        connect_socket(server_socket_fd, &server_addr);
//        bind_ip6_socket(server_socket_fd, input_params.server_port);
        gui_socket_fd = open_udp_ip6_socket();
        bind_ip6_socket(gui_socket_fd, input_params.port);
        INFO("Connected to GUI");
    }

    ~Client() {
        close(server_socket_fd);
        close(gui_socket_fd);
    }

    void run();
};


#endif //CLIENT_H
