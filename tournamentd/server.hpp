#pragma once
#include "socket.hpp"
#include <functional>
#include <iostream>
#include <set>

class server
{
    inet_socket listener;
    std::set<inet_socket> all_open;
    std::set<inet_socket> all_clients;

    // disconnect a client
    void disconnect(const inet_socket& client);

public:
    // create the server, listening on given port
    server(std::uint16_t port);

    // poll the server with given timeout, handling both new clients and clients with input
    bool poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec=-1);

    // call back handler for each client
    void each_client(const std::function<bool(std::ostream&)>& handler);
};