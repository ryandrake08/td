#pragma once
#include "socket.hpp"
#include <functional>
#include <iostream>
#include <set>
#include <string>

class server
{
    std::set<common_socket> all;
    std::set<common_socket> listeners;
    std::set<common_socket> clients;

public:
    // listen on given port
    void listen(const char* service);

    // poll the server with given timeout, handling both new clients and clients with input
    bool poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec=-1);

    // broadcast message to all clients
    void broadcast(const std::string& message) const;
};