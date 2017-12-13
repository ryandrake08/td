#pragma once
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include "json.hpp"
#include "datetime.hpp"

namespace td
{
    class protocol_error : public std::runtime_error
    {
    public:
        explicit protocol_error(const char* what_arg) : std::runtime_error(what_arg) {}
    };

    // key into the player map
    typedef std::string player_id_t;

    // index into the funding_source vector
    typedef size_t funding_source_id_t;

    // type of funding source (buyin, rebuy, addon)
    enum funding_source_type_t { buyin, rebuy, addon };

    // attributes of an authorized client
    struct authorized_client
    {
        int code;
        std::string name;
        datetime added_at;

        authorized_client(int c);
        authorized_client(const json& obj);
    };

    // attributes of a single blind level
    struct blind_level
    {
        std::string game_name;
        unsigned long little_blind;
        unsigned long big_blind;
        unsigned long ante;
        long duration;
        long break_duration;
        std::string reason;

        blind_level();
        blind_level(const json& obj);
    };

    // attributes of a single chip denomination
    struct chip
    {
        std::string color;
        unsigned long denomination;
        unsigned long count_available;

        chip();
        chip(const json& obj);
    };

    // attributes of each funding source (buy-in, addon, etc.)
    struct funding_source
    {
        std::string name;
        funding_source_type_t type;
        std::size_t forbid_after_blind_level;
        unsigned long chips;
        double cost;
        double commission;
        double equity;
        // currency names use ISO 4217
        std::string cost_currency;
        std::string commission_currency;
        std::string equity_currency;

        funding_source();
        funding_source(const json& obj);
    };

    // attributes of each player
    struct player
    {
        player_id_t player_id;
        std::string name;
        datetime added_at;

        player();
        player(const json& obj);
    };

    // attributes of a single physical seat at the tournament
    struct seat
    {
        std::size_t table_number;
        std::size_t seat_number;

        seat();
        seat(const json& obj);
        seat(std::size_t t, std::size_t s);
    };

    // represents a player's movement from one seat to another
    struct player_movement
    {
        player_id_t player_id;
        std::string name;
        seat from_seat;
        seat to_seat;

        player_movement();
        player_movement(const json& obj);
        player_movement(const player_id_t& p, const std::string& n, const seat& f, const seat& t);
    };

    // represents a quantity of chips distributed to each player
    struct player_chips
    {
        unsigned long denomination;
        unsigned long chips;

        player_chips();
        player_chips(const json& obj);
        player_chips(unsigned long d, unsigned long c);
    };

    // represents a manually built payout structure
    struct manual_payout
    {
        size_t buyins_count;
        std::vector<double> payouts;

        manual_payout();
        manual_payout(const json& obj);
        manual_payout(size_t c, const std::vector<double>& p);
    };

    // represents a tournament result
    struct result
    {
        size_t place;
        std::string name;
        double payout;
        std::string payout_currency;

        result();
        result(size_t p, const std::string& n="--");
    };

    // represents a player with additional buyin/seat info
    struct seated_player
    {
        player_id_t player_id;
        std::string name;
        bool buyin;
        std::size_t table_number;
        std::size_t seat_number;

        seated_player();
        seated_player(const player_id_t& p, const std::string& n, bool b);
        seated_player(const player_id_t& p, const std::string& n, bool b, std::size_t t, std::size_t s);
    };
}
