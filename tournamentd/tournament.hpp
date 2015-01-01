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

    // command handlers available to anyone
    void handle_cmd_version(json& out) const;
    void handle_cmd_get_config(json& out) const;
    void handle_cmd_get_state(json& out) const;

    // command handlers available to authorized clients
    void handle_cmd_authorize(const json& in, json& out);
    void handle_cmd_start_game(const json& in, json& out);
    void handle_cmd_stop_game(const json& in, json& out);
    void handle_cmd_resume_game(const json& in, json& out);
    void handle_cmd_pause_game(const json& in, json& out);
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
    // tournament API
#if 0 // How to do these?
    void version(json& out) const;
    void get_config(json& out) const;
    void get_state(json& out) const;
#endif
    int authorize(int code);
    void start_game(const datetime& start_at);
    void start_game();
    void stop_game();
    void resume_game();
    void pause_game();
    std::size_t set_previous_level();
    std::size_t set_next_level();
    void set_action_clock(long duration_ms);
    void reset_action_clock();
    void gen_blind_levels(std::size_t count, long level_duration_ms);
    void reset_funding();
    void fund_player(player_id player, funding_source_id source);
    std::size_t plan_seating(std::size_t max_expected_players);
    gameseating::seat seat_player(player_id player);
    std::vector<gameseating::player_movement> bust_player(player_id player);

    // listen for clients on given port
    void listen(const char* service);

    // load configuration from file
    void load_configuration(const std::string& filename);

    // Run one iteration of the tournament run loop
    bool run();
};