#pragma once
#include "socket.hpp"
#include <set>

class server
{
    inet_socket listener;
    std::set<inet_socket> all_open;

public:
    // create the server, listening on given port
    server(uint16_t port);

    // poll the server with given timeout
    void poll(long usec=-1);
};