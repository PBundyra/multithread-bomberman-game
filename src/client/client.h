#ifndef CLIENT_H
#define CLIENT_H

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
//    boost::asio::ip::address tcp_addr;
//    boost::asio::ip::address udp_addr;
} input_params_t;

input_params_t parse_cli_params(int argc, char **argv);

void print_cli(input_params_t &params);


class Client {
private:
//    unique_ptr<Server> server;
    Buffer buf_server_to_gui;
    Buffer buf_gui_to_server;
    Map map;
//    Lobby lobby;
    atomic_bool is_game_started;
    string name;


    using list_len_t = uint32_t;
    using map_len_t = uint32_t;

    void parse_events(vector<shared_ptr<Event>> &events);

    void parse_hello(const char *msg, size_t len);

    void connect_to_gui(const input_params_t &params);

    void connect_to_server(const input_params_t &params);

public:
    Client(input_params_t &input_params) : name(input_params.player_name) {
        is_game_started = false;
    }

    void run(input_params_t &params);
};



#endif //CLIENT_H
