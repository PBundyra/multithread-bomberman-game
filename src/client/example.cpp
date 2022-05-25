//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

class tcp_connection
        : public boost::enable_shared_from_this<tcp_connection> {
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
    tcp_connection(boost::asio::io_context &io_context)
            : socket_(io_context) {
    }

    void handle_write() {
    }

    tcp::socket socket_;
    std::string message_;
};

class tcp_server {
public:
    tcp_server(boost::asio::io_context &io_context)
            : io_context_(io_context),
              socket_(io_context, tcp::endpoint(tcp::v4(), 1300)) {
//              acceptor_(io_context, tcp::endpoint(tcp::v6(), 1300)) {
        start_accept();
    }

private:
    boost::asio::io_context &io_context_;
//    tcp::acceptor acceptor_;
    tcp::socket socket_;
    char tcp_buffer[1024];

    void start_accept() {
        tcp_connection::pointer new_connection =
                tcp_connection::create(io_context_);

        socket_.async_read_some(boost::asio::buffer(tcp_buffer),
                               boost::bind(&tcp_server::handle_accept, this, new_connection,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
                       const boost::system::error_code &error) {
//        if (!error) {
//            new_connection->start();
//        }
        tcp_buffer[0] = 'e';
        tcp_buffer[1] = 's';
        tcp_buffer[2] = 's';
        tcp_buffer[3] = 'a';

        boost::asio::async_write(socket_, boost::asio::buffer(tcp_buffer),
                                 boost::bind(&tcp_server::handle_accept, this, new_connection,
                                             boost::asio::placeholders::error));
//        socket_.async_write_some(boost::asio::buffer(tcp_buffer, 4), handler_some);

        start_accept();
    }

    void handler_some(const boost::system::error_code &error, size_t bytes_transferred) {
        std::cout << "handler_some\n";
    }

};

class udp_server {
public:
    udp_server(boost::asio::io_context &io_context)
            : socket_(io_context, udp::endpoint(udp::v6(), 1300)) {
        start_receive();
    }

private:
    void start_receive() {
        socket_.async_receive_from(
                boost::asio::buffer(recv_buffer_), remote_endpoint_,
                boost::bind(&udp_server::handle_receive, this,
                            boost::asio::placeholders::error));
    }

    void handle_receive(const boost::system::error_code &error) {
        if (!error) {
            boost::shared_ptr<std::string> message(
                    new std::string(make_daytime_string()));

            socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
                                  boost::bind(&udp_server::handle_send, this, message));
            std::cout << "Wyslalem po udp\n";

            start_receive();
        }
    }

    void handle_send(boost::shared_ptr<std::string> /*message*/) {
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, 1> recv_buffer_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        tcp_server server1(io_context);
        udp_server server2(io_context);
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}