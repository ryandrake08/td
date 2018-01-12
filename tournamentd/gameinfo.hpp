#pragma once
#include "types.hpp"
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class json;

class gameinfo
{
    // ----- configuration -----

    // configuration: human-readable name of this tournament
    std::string name;

    // configuration: list of all known players (playing or not)
    std::unordered_map<td::player_id_t,td::player> players;

    // configuration: number of players per table
    std::size_t table_capacity;

    // configuration: rough percentage of seats that get paid (0.0-1.0)
    double percent_seats_paid;

    // configuration: round to whole numbers when calculating payouts?
    bool round_payouts;

    // configuration: payout structure flatness
    double payout_flatness;

    // configuration: funding rules
    std::vector<td::funding_source> funding_sources;

    // configuration: blind structure for this game
    std::vector<td::blind_level> blind_levels;

    // configuration: description of each chip (for display)
    std::vector<td::chip> available_chips;

    // configuration: manually generated payout structures
    std::unordered_map<size_t, std::vector<double>> manual_payouts;

    // configuration: how long after round starts should prev command go to the previous round (rather than restart)? (ms)
    long previous_blind_level_hold_duration;

    // configuration: clock screen background color (synced to all clients)
    std::string background_color;

    // ----- state -----

    // players seated in the game
    std::unordered_map<td::player_id_t,td::seat> seats;

    // players without seats or busted out
    std::deque<td::player_id_t> players_finished;

    // maximum number of expected players
    std::size_t max_expected_players;

    // empty seats
    std::deque<td::seat> empty_seats;

    // number of tables total
    std::size_t tables;

    // players who bought in at least once
    std::unordered_set<td::player_id_t> buyins;

    // list of each entry (buyin or rebuy)
    std::deque<td::player_id_t> entries;

    // payout structure
    std::vector<double> payouts;

    // total game currency (chips) in play
    unsigned long total_chips;

    // total funds received and paid out, for each currency
    std::unordered_map<std::string,double> total_cost;
    std::unordered_map<std::string,double> total_commission;
    std::unordered_map<std::string,double> total_equity;

    // is the game running or paused?
    bool running;

    // Current bind level (index into above vector)
    // Note: blind level 0 is reserved for setup/planning
    std::size_t current_blind_level;

    // represents a point in time
    typedef std::chrono::system_clock::time_point time_point_t;

    // represents a time duration
    typedef std::chrono::milliseconds duration_t;

    // end of period (valid when running)
    time_point_t end_of_round;
    time_point_t end_of_break;

    // ms remaining
    duration_t time_remaining;
    duration_t break_time_remaining;

    // action clock
    time_point_t end_of_action_clock;
    duration_t action_clock_time_remaining;

    // elapsed time
    time_point_t tournament_start;
    duration_t elapsed_time;

    // ----- utility -----

    // calculate derived state and dump to JSON
    void dump_derived_state(json& state) const;

    // utility: return a description of a player, by id
    const std::string player_description(const td::player_id_t& player_id) const;

    // utility: arrange tables with lists of players
    std::vector<std::vector<td::player_id_t> > players_at_tables() const;

    // return the maximum number of chips available per player for a given denomination
    size_t max_chips_for(unsigned long denomination, std::size_t players_count) const;

    // reset seating to an empty, unplanned game
    void reset_seating();

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

    // utility: start a blind level
    void start_blind_level(std::size_t blind_level, duration_t offset);

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

    // ----- funding -----

    // fund a player, (re-)buyin or addon
    void fund_player(const td::player_id_t& player_id, const td::funding_source_id_t& src);

    // calculate number of chips per denomination for this funding source, given totals and number of players
    std::vector<td::player_chips> chips_for_buyin(const td::funding_source_id_t& src, std::size_t max_expected_players) const;

    // reset game state
    void reset_funding();

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

    // update time remaining
    bool update_remaining();

    // set the action clock (when someone 'needs the clock called on them'
    void set_action_clock(long duration);

    // reset the action clock
    void reset_action_clock();

    // generate progressive blind levels, given available chip denominations
    void gen_blind_levels(std::size_t count, long level_duration, long chip_up_break_duration, double blind_increase_factor);
};
