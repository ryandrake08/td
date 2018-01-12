#pragma once
#include "datetime.hpp"
#include "gameinfo.hpp"
#include "server.hpp"
#include "types.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

class tournament
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    tournament();
    ~tournament();

    // authorize a client code
    int authorize(int code);

    // listen for clients on any available service, returning the unix socket path and port
    std::pair<std::string, int> listen(const char* unix_socket_directory);

    // load configuration from file
    void load_configuration(const std::string& filename);

    // Run one iteration of the tournament run loop
    bool run();
};