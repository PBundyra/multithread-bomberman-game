#ifndef CLIENT_H
#define CLIENT_H

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/bind/bind.hpp>
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
#include "common.h"
#include "err.h"

using port_t = uint16_t;

typedef struct input_params_t {
    port_t port;
    string player_name;
    port_t gui_port;
    string gui_host;
    port_t server_port;
    string server_host;
    struct sockaddr_in server_addr;
    struct sockaddr_in gui_addr;
} input_params_t;

input_params_t parse_cli_params(int argc, char **argv);

void print_cli(input_params_t &params);

int bind_socket(uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    ENSURE(socket_fd > 0);
    // after socket() call; we should close(sock) on any execution path;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port);

    // bind the socket to a concrete address
    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &server_address,
                     (socklen_t) sizeof(server_address)));

    return socket_fd;
}


class Client {
private:
    using list_len_t = uint32_t;
    using map_len_t = uint32_t;


    port_t port;
    string player_name;
    port_t gui_port;
    string gui_host;
    port_t server_port;
    string server_host;
    struct sockaddr_in server_addr;
    struct sockaddr_in gui_addr;

    Buffer buf_server_to_gui;
    Buffer buf_gui_to_server;
    Map map;
//    Lobby lobby;
    atomic_bool is_game_started;
    string name;
    int tcp_socket_fd;
    int udp_socket_fd;

    void parse_hello(const char *msg, const size_t msg_len);

    size_t get_msg_from_gui();

    void send_msg_to_gui();

    void parse_msg_from_gui(const size_t msg_len);

    size_t get_msg_from_server();

    void send_msg_to_server();

    void parse_msg_from_server();

    void gui_to_server_handler();

    void server_to_gui_handler();

    void receive_hello();

public:
    Client(input_params_t &input_params) : name(input_params.player_name), port(input_params.port),
                                           gui_port(input_params.gui_port), gui_host(input_params.gui_host),
                                           server_port(input_params.server_port), server_host(input_params.server_host),
                                           server_addr(input_params.server_addr), gui_addr(input_params.gui_addr),
                                           player_name(input_params.player_name), is_game_started(false) {
        cout << "Player " << player_name << " connected to server" << endl;
        tcp_socket_fd = open_tcp_socket();
        connect_socket(tcp_socket_fd, &input_params.server_addr);
        udp_socket_fd = bind_socket(port);
    }

    ~Client() {
        close(tcp_socket_fd);
        close(udp_socket_fd);
    }

    void run(input_params_t &params);
};

#endif //CLIENT_H
