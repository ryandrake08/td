#pragma once
#include "datetime.hpp"
#include "json.hpp"
#include <deque>
#include <string>
#include <unordered_set>
#include <vector>

class tournament
{
    typedef size_t currency;

    enum event_t
    {
        did_buyin,
        did_rebuy,
        did_addon,
        did_change_seat,
        did_bust
    };

    enum game_state_t
    {
        planning,
        running,
        paused
    };

    struct funding_source
    {
        bool allowed;
        size_t forbid_after_round;
        size_t chips;
        currency cost;
        currency commission;
        currency equity;
        static funding_source load(const json& config);
    };

    struct player
    {
        std::string name;
        static player load(const json& config);
    };

    struct chip
    {
        std::string color;
        size_t denomination;
        size_t count_available;
        static chip load(const json& config);
    };

    struct blind_level
    {
        size_t little_blind;
        size_t big_blind;
        size_t ante;
        int duration_ms;
        int break_duration_ms;
        static blind_level load(const json& config);
    };

    struct seat
    {
        size_t player;
        size_t table_number;
        size_t seat_number;
        static seat load(const json& config);
    };

    struct payout
    {
        bool derive_award;
        size_t percent_x100;
        currency award;
        static payout load(const json& config);
    };

    struct activity
    {
        event_t event;
        size_t player;
        datetime timestamp;
    };

    struct configuration
    {
        std::string name;
        std::string cost_currency;
        std::string equity_currency;
        size_t table_capacity;
        funding_source buyin;
        funding_source rebuy;
        funding_source addon;
        std::vector<player> players;
        std::vector<chip> chips;
        std::vector<blind_level> blind_levels;
        std::vector<seat> seats;
        std::vector<payout> payouts;
        static configuration load(const json& config);
    };

    // game configuration
    configuration cfg;

    // game state and blind level
    game_state_t state;
    size_t current_blind_level;

    // time
    datetime checkpoint;
    int time_remaining_ms;
    int break_time_remaining_ms;

    // players
    std::unordered_set<size_t> players_playing;
    std::deque<size_t> players_finished;

    // totals
    size_t total_chips;
    currency total_cost;
    currency total_commission;
    currency total_equity;

    // activity log
    std::vector<activity> activity_log;

public:
    // construct a tournament, loading configuration from JSON (object or file)
    explicit tournament(const json& cfg);
};