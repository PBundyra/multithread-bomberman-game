#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using namespace std;

class Player {
private:
    string name;
    asio::ip::address address;
public:
    Player(string name, asio::ip::address address) : name(name), address(address) {};

};

#endif //PLAYER_H
