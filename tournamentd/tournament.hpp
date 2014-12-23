#pragma once
#include "datetime.hpp"
#include "json.hpp"
#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class tournament
{
public:
    class exception : public std::runtime_error
    {
    public:
        explicit exception(const std::string& what_arg);
        explicit exception(const char* what_arg);
    };

    typedef std::size_t currency;
    typedef std::size_t player_id;

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
        int duration_ms;
        int break_duration_ms;
    };

    struct seat
    {
        std::size_t table_number;
        std::size_t seat_number;
    };

    struct payout
    {
        // if true, the percent field is user-set and award is derived
        bool derive_award;
        std::size_t percent_x100;
        currency award;
    };

    struct activity
    {
        enum event_t
        {
            did_buyin,
            did_addon,
            did_seat,
            did_change_seat,
            did_bust
        };
        
        event_t event;
        player_id player;
        datetime timestamp;
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

        // funding rules
        std::vector<funding_source> funding_sources;

        // list of all known players (playing or not)
        std::vector<player> players;

        // description of each chip (for display)
        std::vector<chip> chips;

        // blind structure for this game
        std::vector<blind_level> blind_levels;

        // payout structure
        std::vector<payout> payouts;
    };

    struct player_movement
    {
        player_id player;
        seat from_seat;
        seat to_seat;
    };

private:
    // game configuration
    configuration cfg;

    // is the game running or paused?
    bool running;

    // Current bind level (index into vector)
    // Note: blind level -1 is reserved for setup/planning
    std::size_t current_blind_level;

    // last time checkpoint (checkpoints happen when timer stops or starts)
    datetime checkpoint;

    // ms remaining after last checkpoint
    int time_remaining_ms;
    int break_time_remaining_ms;

    // players seated in the game
    std::unordered_map<player_id,seat> seats;

    // players who bought in at least once
    std::unordered_set<player_id> buyins;

    // players without seats or busted out
    std::deque<player_id> players_finished;

    // empty seats
    std::deque<seat> empty_seats;

    // number of tables total
    std::size_t tables;

    // total game currency (chips) in play
    std::size_t total_chips;

    // total funds received
    currency total_cost;
    currency total_commission;

    // total funds to pay out
    currency total_equity;

    // activity log
    std::vector<activity> activity_log;

    // log an activity
    void log(activity::event_t ev, player_id player);

    // utility: arrange tables with lists of players
    std::vector<std::vector<player_id>> players_at_tables() const;

public:
    // load configuration from JSON (object or file)
    void configure(const json& config);

    // reset game back to planning state
    void reset();

    // has the game started?
    bool is_started() const;

    // start the game
    void start();


    // ----- seating -----

    // pre-game player seeting, with expected number of players (to predict table count)
    // returns number of tables needed
    std::size_t plan_seating(std::size_t max_expected_players);

    // add player to an existing game
    // returns player's seat
    seat add_player(const player_id& player);

    // fund a player, (re-)buyin or addon
    void fund_player(const player_id& player, const funding_source& source);

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

};