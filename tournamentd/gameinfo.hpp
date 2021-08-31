#pragma once
#include "nlohmann/json_fwd.hpp"
#include "types.hpp"
#include <memory>
#include <string>
#include <vector>

class datetime;

class gameinfo
{
    // pimpl
    class impl;
    std::unique_ptr<impl> pimpl;

public:
    gameinfo();
    ~gameinfo();

    // load configuration from JSON (object or file)
    void configure(const nlohmann::json& config);

    // dump configuration to JSON
    void dump_configuration(nlohmann::json& config) const;

    // dump state to JSON
    void dump_state(nlohmann::json& state) const;

    // some configuration gets sent to clients along with state, dump to JSON
    void dump_configuration_state(nlohmann::json& state) const;

    // calculate derived state and dump to JSON
    void dump_derived_state(nlohmann::json& state) const;

    // has internal state been updated since last check?
    bool state_is_dirty();

    // reset game state
    void reset_state();

    // ----- seating -----

    // pre-game player seeting, with expected number of players (to predict table count)
    std::vector<td::player_movement> plan_seating(std::size_t max_expected);

    // add player to an existing game, returning a message (either player_seated or already_seated) and player's seat
    // returns player's seat
    std::pair<std::string, td::seated_player> add_player(const td::player_id_t& player_id);

    // remove a player from the game (as though player never existed in the game), returning a message (player_unseated) and player's seat removed from
    void remove_player(const td::player_id_t& player_id);

    // remove a player from the game, busting him out
    // returns any player movements that happened
    std::vector<td::player_movement> bust_player(const td::player_id_t& player_id);

    // try to break and rebalance tables
    // returns description of movements
    std::vector<td::player_movement> rebalance_seating();

    // ----- funding -----

    // fund a player, (re-)buyin or addon
    void fund_player(const td::player_id_t& player_id, const td::funding_source_id_t& src);

    // calculate number of chips per denomination for this funding source, given totals and number of players
    std::vector<td::player_chips> chips_for_buyin(const td::funding_source_id_t& src, std::size_t max_expected) const;

    // ----- both seating and funding -----

    // quickly set up a game (plan, seat, and buyin, using optional funding source)
    // returns all seated players
    std::vector<td::seated_player> quick_setup();
    std::vector<td::seated_player> quick_setup(const td::funding_source_id_t& src);

    // ----- clock -----

    // has the game started?
    bool is_started() const;

    // start the game (optionally at certain time);
    void start();
    void start(const datetime& starttime);

    // stop the game
    void stop();

    // pause
    void pause();

    // resume
    void resume();

    // toggle pause/remove
    void toggle_pause_resume();

    // advance to next blind level
    // returns: true if blind level changed, false if we are at end of levels
    bool next_blind_level();

    // return to previous blind level
    // returns: true if blind level changed, false if blind level was just reset
    bool previous_blind_level();

    // update game state
    void update();

    // set the action clock (when someone 'needs the clock called on them'
    void set_action_clock(long duration_milliseconds);

    // reset the action clock
    void reset_action_clock();

    // generate progressive blind levels, given desired duration and starting stacks
    std::vector<td::blind_level> gen_blind_levels(long desired_duration_milliseconds, long level_duration_milliseconds, std::size_t expected_buyins, std::size_t expected_rebuys, std::size_t expected_addons, long chip_up_break_duration_milliseconds, td::ante_type_t antes, double ante_sb_ratio) const;
};
