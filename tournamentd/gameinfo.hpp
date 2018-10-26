#pragma once
#include "types.hpp"
#include <deque>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class json;

class gameinfo
{
    // ----- random number engine -----

    std::default_random_engine random_engine;

    // ----- configuration -----

    // configuration: human-readable name of this tournament
    std::string name;

    // configuration: list of all known players (playing or not)
    std::unordered_map<td::player_id_t,td::player> players;

    // configuration: number of players per table
    std::size_t table_capacity;

    // configuration: funding rules
    std::vector<td::funding_source> funding_sources;

    // configuration: blind structure for this game
    std::vector<td::blind_level> blind_levels;

    // configuration: description of each chip (for display)
    std::vector<td::chip> available_chips;

    // configuration: payout policy
    td::payout_policy_t payout_policy;

    // configuration: payout currency
    std::string payout_currency;

    // configuration: automatic payout parameters
    td::automatic_payout_parameters automatic_payouts;

    // configuration: forced payout structure (regardless of number of players)
    std::vector<td::monetary_value_nocurrency> forced_payouts;

    // configuration: manually generated payout structures
    std::unordered_map<size_t, std::vector<td::monetary_value_nocurrency>> manual_payouts;

    // configuration: how long after round starts should prev command go to the previous round (rather than restart)? (ms)
    long previous_blind_level_hold_duration;

    // configuration: rebalance policy
    td::rebalance_policy_t rebalance_policy;

    // configuration: clock screen background color (synced to all clients)
    std::string background_color;

    // ----- state -----

    // state is dirty
    bool dirty;

    // ---------- results ----------

    // finished out players in reverse order of bust out
    std::deque<td::player_id_t> players_finished;

    // busted out players in order of bust out
    // TODO: make this a pair (buster and busted) to support knockout tournaments
    std::deque<td::player_id_t> bust_history;

    // ---------- seating ----------

    // players seated in the game
    std::unordered_map<td::player_id_t,td::seat> seats;

    // empty seats
    std::deque<td::seat> empty_seats;

    // number of tables total
    std::size_t tables;

    // ---------- funding ----------

    // players who are both currently seated and bought in
    std::unordered_set<td::player_id_t> buyins;

    // players who at one point have bought in
    std::unordered_set<td::player_id_t> unique_entries;

    // ordered list of each entry (buyin or rebuy)
    std::deque<td::player_id_t> entries;

    // payout structure
    std::vector<td::monetary_value> payouts;

    // total game currency (chips) in play
    unsigned long total_chips;

    // total funds received, for each currency
    std::unordered_map<std::string,double> total_cost;
    std::unordered_map<std::string,double> total_commission;

    // total fund paid out, in payout_currency
    double total_equity;

    // ---------- clock ----------

    // Current bind level (index into above vector)
    // Note: blind level 0 is reserved for setup/planning
    std::size_t current_blind_level;

    // represents a point in time
    typedef std::chrono::system_clock::time_point time_point_t;

    // end of period (valid when running)
    time_point_t end_of_round;
    time_point_t end_of_break;

    // action clock
    time_point_t end_of_action_clock;

    // elapsed time
    time_point_t tournament_start;

    // paused time
    time_point_t paused_time;

    // ----- utility -----

    // represents a time duration
    typedef std::chrono::milliseconds duration_t;

    // get current time
    static time_point_t now();

    // utility: return a description of a player, by id
    const std::string player_description(const td::player_id_t& player_id) const;

    // utility: arrange tables with lists of players
    std::vector<std::vector<td::player_id_t> > players_at_tables() const;

    // return the maximum number of chips available per player for a given denomination
    size_t max_chips_for(unsigned long denomination, std::size_t players_count) const;

    // return a default funding source of the given type. used for quick setup and structure generator
    td::funding_source_id_t source_for_type(const td::funding_source_type_t& type) const;

    // move a player to a specific table
    // returns player's original seat and new seat
    td::player_movement move_player(const td::player_id_t& player_id, std::size_t table);

    // move a player to the table with the smallest number of players, optionally avoiding a particular table
    // returns player's movement
    td::player_movement move_player(const td::player_id_t& player_id, const std::unordered_set<std::size_t>& avoid_tables);

    // re-balance by moving any player from a large table to a smaller one
    // returns number of movements, or zero, if no players moved
    std::size_t try_rebalance(std::vector<td::player_movement>& movements);

    // break a table if possible
    // returns number of movements, or zero, if no players moved
    std::size_t try_break_table(std::vector<td::player_movement>& movements);

    // re-calculate payouts
    void recalculate_payouts();

    // is paused
    bool is_paused() const;

    // utility: start a blind level
    void start_blind_level(std::size_t blind_level, duration_t offset);

    // utility: generate a number of progressive blind levels, given increase factor
    std::vector<td::blind_level> gen_count_blind_levels(std::size_t count, long level_duration, long chip_up_break_duration, double blind_increase_factor, td::ante_type_t antes, double ante_sb_ratio) const;

public:
    // initialize game
    gameinfo();

    // validate gameinfo structure
    void validate();

    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;

    // dump state to JSON
    void dump_state(json& state) const;

    // some configuration gets sent to clients along with state, dump to JSON
    void dump_configuration_state(json& state) const;

    // calculate derived state and dump to JSON
    void dump_derived_state(json& state) const;

    // has internal state been updated since last check?
    bool state_is_dirty();

    // reset game state
    void reset_state();

    // ----- seating -----

    // pre-game player seeting, with expected number of players (to predict table count)
    // returns number of tables needed
    std::size_t plan_seating(std::size_t max_expected_players);

    // add player to an existing game, returning player's seat
    // returns player's seat
    std::pair<std::string, td::seat> add_player(const td::player_id_t& player_id);

    // remove a player from the game (as though player never existed in the game), returning seat removed from
    td::seat remove_player(const td::player_id_t& player_id);

    // remove a player from the game, busting him out
    // returns any player movements that happened
    std::vector<td::player_movement> bust_player(const td::player_id_t& player_id);

    // try to break and rebalance tables
    std::vector<td::player_movement> rebalance_seating();

    // ----- funding -----

    // fund a player, (re-)buyin or addon
    void fund_player(const td::player_id_t& player_id, const td::funding_source_id_t& src);

    // calculate number of chips per denomination for this funding source, given totals and number of players
    std::vector<td::player_chips> chips_for_buyin(const td::funding_source_id_t& src, std::size_t max_expected_players) const;

    // ----- both planning and seating -----

    // quickly set up a game (plan, seat, and buyin, using optional funding source)
    std::vector<td::seated_player> quick_setup();
    std::vector<td::seated_player> quick_setup(const td::funding_source_id_t& src);

    // ----- clock -----

    // has the game started?
    bool is_started() const;

    // return current blind level
    std::size_t get_current_blind_level() const;

    // start the game (optionally at certain time);
    void start();
    void start(const time_point_t& starttime);

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
    bool next_blind_level(duration_t offset=duration_t::zero());

    // return to previous blind level
    // returns: true if blind level changed, false if blind level was just reset
    bool previous_blind_level(duration_t offset=duration_t::zero());

    // update game state
    void update();

    // set the action clock (when someone 'needs the clock called on them'
    void set_action_clock(long duration);

    // reset the action clock
    void reset_action_clock();

    // generate progressive blind levels, given desired duration and starting stacks
    std::vector<td::blind_level> gen_blind_levels(long desired_duration, long level_duration, std::size_t expected_buyins, std::size_t expected_rebuys, std::size_t expected_addons, long chip_up_break_duration, td::ante_type_t antes, double ante_sb_ratio) const;
};
