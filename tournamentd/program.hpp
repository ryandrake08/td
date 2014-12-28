#pragma once
#include <string>
#include <unordered_set>
#include <vector>
#include "server.hpp"
#include "gameinfo.hpp"

class program
{
    gameinfo game;
    server sv;

    // accepted authorization codes
    std::unordered_set<int> auths;

    // throw if unauthorized
    void ensure_authorized(const json& in);

    // handler for new client
    bool handle_new_client(std::ostream& client);

    // handler for input from existing client
    bool handle_client_input(std::iostream& client);

    // handler for async game events
    bool handle_game_event(std::ostream& client);

public:
    // Create a program, given command line options
    explicit program(const std::vector<std::string>& cmdline);

    // Run one iteration of the program run loop
    bool run();
};