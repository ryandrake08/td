#pragma once
#include "gameinfo.hpp"
#include "server.hpp"
#include <unordered_set>

struct tournament
{
    // game objects
    gameinfo game_info;

    // server to handle remote connections
    server sv;

    // accepted authorization codes
    std::unordered_set<int> auths;

private:
    // throw if unauthorized
    void ensure_authorized(const json& in);

    // handler for new client
    bool handle_new_client(std::ostream& client);

    // handler for input from existing client
    bool handle_client_input(std::iostream& client);

    // handler for async game events
    bool handle_game_event(std::ostream& client);

public:
    // 
    // Run one iteration of the tournament run loop
    bool run();
};