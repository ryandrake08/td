#pragma once
#include "datetime.hpp"
#include "gameinfo.hpp"
#include "gameclock.hpp"
#include "gamefunding.hpp"
#include "gameseating.hpp"
#include "server.hpp"
#include "types.hpp"
#include <string>
#include <iostream>
#include <unordered_set>

class tournament
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

    // auth check
    void ensure_authorized(const json& in) const;

    // broadcast helpers;
    template <typename T>
    void broadcast_state(const T& object) const;
    
    template <typename T>
    void broadcast_configuration(const T& object) const;

    // command handlers available to anyone
    void handle_cmd_version(json& out) const;
    void handle_cmd_get_config(json& out) const;
    void handle_cmd_get_state(json& out) const;
    void handle_cmd_check_authorized(const json& in, json& out);

    // command handlers available to authorized clients
    void handle_cmd_authorize(const json& in, json& out);
    void handle_cmd_configure(const json& in, json& out);
    void handle_cmd_start_game(const json& in, json& out);
    void handle_cmd_stop_game(const json& in, json& out);
    void handle_cmd_resume_game(const json& in, json& out);
    void handle_cmd_pause_game(const json& in, json& out);
    void handle_cmd_toggle_pause_game(const json& in, json& out);
    void handle_cmd_set_previous_level(const json& in, json& out);
    void handle_cmd_set_next_level(const json& in, json& out);
    void handle_cmd_set_action_clock(const json& in, json& out);
    void handle_cmd_reset_action_clock(const json& in, json& out);
    void handle_cmd_gen_blind_levels(const json& in, json& out);
    void handle_cmd_reset_funding(const json& in, json& out);
    void handle_cmd_fund_player(const json& in, json& out);
    void handle_cmd_plan_seating(const json& in, json& out);
    void handle_cmd_seat_player(const json& in, json& out);
    void handle_cmd_bust_player(const json& in, json& out);

    // handler for new client
    bool handle_new_client(std::ostream& client) const;

    // handler for input from existing client
    bool handle_client_input(std::iostream& client);

public:
    // authorize a client code
    int authorize(int code);

    // listen for clients on given port
    void listen(const char* unix_socket_path, const char* service);

    // load configuration from file
    void load_configuration(const std::string& filename);

    // Run one iteration of the tournament run loop
    bool run();
};