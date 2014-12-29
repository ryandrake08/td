#pragma once
#include "gameinfo.hpp"
#include "server.hpp"
#include <string>
#include <iostream>
#include <unordered_set>

struct tournament
{
    // game objects
    gameinfo game_info;

    // server to handle remote connections
    server game_server;

    // accepted authorization codes
    std::unordered_set<int> game_auths;

private:
    // handler for new client
    bool handle_new_client(std::ostream& client);

    // handler for input from existing client
    bool handle_client_input(std::iostream& client);

    // handler for async game events
    bool handle_game_event(std::ostream& client);

public:
    // load configuration from file
    void load_configuration(const std::string& filename);

    // Run one iteration of the tournament run loop
    bool run();
};