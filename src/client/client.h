#ifndef CLIENT_H
#define CLIENT_H

#include "buffer.h"
#include "utils.h"
#include "common.h"
#include "game.h"

typedef struct input_params_t {
    port_t port;
    std::string player_name;
    port_t gui_port;
    std::string gui_host;
    port_t server_port;
    std::string server_host;
    struct sockaddr_in server_addr;
    struct sockaddr_in6 gui_addr;
} input_params_t;

input_params_t parse_cli_params(int argc, char **argv);

class Client {
private:
    port_t port;
    std::string player_name;
    port_t gui_port;
    std::string gui_host;
    port_t server_port;
    std::string server_host;
    struct sockaddr_in server_addr;
    struct sockaddr_in6 gui_addr;

    Buffer buf_server_to_gui;
    Buffer buf_gui_to_server;
    Game game;
    std::atomic_bool is_game_started;
    int tcp_socket_fd;
    int udp_socket_fd;

    void handle_hello_msg(Buffer &buf);

    void handle_accepted_player_msg(Buffer &buf);

    void handle_game_started_msg(Buffer &buf);

    void handle_turn_msg(Buffer &buf);

    void handle_game_ended_msg(Buffer &buf);

    size_t get_msg_from_gui();

    void send_msg_to_gui();

    void parse_msg_from_gui(size_t msg_len);

//    size_t get_n_bytes_from_server(void *buffer, size_t n) const;

    void send_msg_to_server();

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
        tcp_socket_fd = open_tcp_socket();
        connect_socket(tcp_socket_fd, &input_params.server_addr);
        INFO("Connected to server");
        udp_socket_fd = bind_udp_socket(port);
        INFO("Connected to GUI");
    }

    ~Client() {
        close(tcp_socket_fd);
        close(udp_socket_fd);
    }

    void run();
};

#endif //CLIENT_H
