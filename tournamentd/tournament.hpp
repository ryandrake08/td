#pragma once
#include "json.hpp"
#include <chrono>
#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class tournament
{
public:

    // ----- types -----

    class exception : public std::runtime_error
    {
    public:
        explicit exception(const std::string& what_arg);
        explicit exception(const char* what_arg);
    };

    typedef std::size_t currency;
    typedef std::size_t player_id;
    typedef std::chrono::system_clock::time_point tp;
    typedef std::chrono::milliseconds ms;

    static const player_id no_player;

    struct funding_source
    {
        static const std::size_t always_allow;

        bool is_addon;
        std::size_t forbid_after_blind_level;
        std::size_t chips;
        currency cost;
        currency commission;
        currency equity;
    };

    struct player
    {
        std::string name;
    };

    struct chip
    {
        std::string color;
        std::size_t denomination;
        std::size_t count_available;
    };

    struct blind_level
    {
        std::size_t little_blind;
        std::size_t big_blind;
        std::size_t ante;
        long duration;
        long break_duration;
    };

    struct seat
    {
        std::size_t table_number;
        std::size_t seat_number;
    };

    struct activity
    {
        enum event_t
        {
            game_reset,
            game_new_blind_level,
            player_did_buyin,
            player_did_addon,
            player_did_seat,
            player_did_change_seat,
            player_did_bust
        };
        
        event_t event;
        player_id player;
        tp timestamp;
    };

    struct player_movement
    {
        player_id player;
        seat from_seat;
        seat to_seat;
    };

    struct configuration
    {
        // game name
        std::string name;

        // name of currency collected (USD, EUR, points)
        std::string cost_currency;

        // name of currency distributed (USD, EUR, points)
        std::string equity_currency;

        // number of players per table
        std::size_t table_capacity;

        // payout rules
        bool payouts_proportional;
        double percent_seats_paid;
        currency guarantee_first;
        currency guarantee_last;

        // funding rules
        std::vector<funding_source> funding_sources;

        // list of all known players (playing or not)
        std::vector<player> players;

        // description of each chip (for display)
        std::vector<chip> chips;

        // blind structure for this game
        std::vector<blind_level> blind_levels;
    };

    // ----- utility -----

    // log an activity
    activity log(activity::event_t ev, player_id player=no_player);


    // ----- general -----

private:
    // game configuration
    configuration cfg;

    // activity log
    std::vector<activity> activity_log;

public:
    // load configuration from JSON (object or file)
    void configure(const json& config);

    // reset game back to planning state
    void reset();

    // ----- clock -----

private:
    // is the game running or paused?
    bool running;

    // Current bind level (index into vector)
    // Note: blind level 0 is reserved for setup/planning
    std::size_t current_blind_level;

    // end of period (valid when running)
    tp end_of_round;
    tp end_of_break;

    // ms remaining
    ms time_remaining;
    ms break_time_remaining;

    // utility: start a blind level
    void start_blind_level(std::size_t blind_level, ms offset);

public:
    // has the game started?
    bool is_started() const;

    // start the game (optionally at certain time);
    void start();
    void start(const tp& starttime);

    // stop the game
    void stop();

    // toggle pause
    void pause();

    // toggle resume
    void resume();

    // advance to next blind level
    bool advance_blind_level(ms offset=ms::zero());

    // restart current blind level
    void restart_blind_level(ms offset=ms::zero());

    // return to previous blind level
    bool previous_blind_level(ms offset=ms::zero());

    // update time remaining
    bool update_remaining();

    // ----- seating -----

private:
    // players seated in the game
    std::unordered_map<player_id,seat> seats;

    // players without seats or busted out
    std::deque<player_id> players_finished;

    // empty seats
    std::deque<seat> empty_seats;

    // number of tables total
    std::size_t tables;

    // utility: arrange tables with lists of players
    std::vector<std::vector<player_id>> players_at_tables() const;

public:
    // pre-game player seeting, with expected number of players (to predict table count)
    // returns number of tables needed
    std::size_t plan_seating(std::size_t max_expected_players);

    // add player to an existing game
    // returns player's seat
    seat add_player(const player_id& player);

    // remove a player from the game
    // returns bust-out order (1 being the first to bust)
    std::size_t remove_player(const player_id& player);

    // move a player to a specific table
    // returns player's original seat and new seat
    player_movement move_player(const player_id& player, std::size_t table);

    // move a player to the table with the smallest number of players, optionally avoiding a particular table
    // returns player's movement
    player_movement move_player(const player_id& player, const std::unordered_set<std::size_t>& avoid_tables);

    // re-balance by moving any player from a large table to a smaller one, returns true if player was moved
    // returns all player movements
    std::vector<player_movement> try_rebalance();

    // break a table if possible
    // returns all player movements
    std::vector<player_movement> try_break_table();

    // ----- chips -----

private:
    // players who bought in at least once
    std::unordered_set<player_id> buyins;

    // payout structure
    std::vector<currency> payouts;

    // total game currency (chips) in play
    std::size_t total_chips;

    // total funds received
    currency total_cost;
    currency total_commission;

    // total funds to pay out
    currency total_equity;

public:
    // fund a player, (re-)buyin or addon
    void fund_player(const player_id& player, const funding_source& source);

    // re-calculate payouts
    std::vector<currency> recalculate_payouts();
};