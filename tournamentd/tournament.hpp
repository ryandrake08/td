#pragma once
#include "gameinfo.hpp"
#include "gameclock.hpp"
#include "gamefunding.hpp"
#include "gameseating.hpp"
#include "server.hpp"
#include <string>
#include <iostream>
#include <unordered_set>

struct tournament
{
    // game objects
    gameinfo game_info;
    gameclock clock;
    gamefunding funding;
    gameseating seating;

    // server to handle remote connections
    server game_server;

    // accepted authorization codes
    std::unordered_set<int> game_auths;

private:
    // auth check
    void ensure_authorized(const json& in) const;

    // command handlers available to anyone
    void handle_cmd_version(json& out) const;
    void handle_cmd_get_config(json& out) const;
    void handle_cmd_get_state(json& out) const;

    // command handlers available to authorized clients
    void handle_cmd_authorize(json& out, const json& in);
    void handle_cmd_start_game(const json& in);
    void handle_cmd_stop_game(const json& in);
    void handle_cmd_resume_game(const json& in);
    void handle_cmd_pause_game(const json& in);
    void handle_cmd_previous_level(const json& in);
    void handle_cmd_next_level(const json& in);
    void handle_cmd_set_action_clock(const json& in);
    void handle_cmd_gen_blind_levels(const json& in);

    // handler for new client
    bool handle_new_client(std::ostream& client) const;

    // handler for input from existing client
    bool handle_client_input(std::iostream& client);

    // handler for async game events
    bool handle_game_event(std::ostream& client) const;

public:
    // load configuration from file
    void load_configuration(const std::string& filename);

    // Run one iteration of the tournament run loop
    bool run();
};