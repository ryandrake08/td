#pragma once
#include "socket.hpp"
#include <functional>
#include <iostream>
#include <set>

class server
{
    inet_socket listener;
    std::set<inet_socket> all_open;

public:
    // create the server, listening on given port
    server(std::uint16_t port);

    typedef std::function<void(std::iostream&)> handler;

    // poll the server with given timeout
    bool poll(const handler& handle_new_client, const handler& handle_client, long usec=-1);
};