#pragma once
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include "json.hpp"
#include "datetime.hpp"

namespace td
{
    class runtime_error : public std::runtime_error
    {
    public:
        explicit runtime_error(const char* what_arg) : std::runtime_error(what_arg) {}
    };

    // key into the player map
    typedef std::size_t player_id_t;

    // index into the funding_source vector
    typedef std::size_t funding_source_id_t;

    // attributes of a single blind level
    struct blind_level
    {
        unsigned long little_blind;
        unsigned long big_blind;
        unsigned long ante;
        long duration;
        long break_duration;

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
        bool is_addon;
        std::size_t forbid_after_blind_level;
        unsigned long chips;
        double cost;
        double commission;
        double equity;

        funding_source();
        funding_source(const json& obj);
    };

    // attributes of each player
    struct player
    {
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
        seat from_seat;
        seat to_seat;

        player_movement();
        player_movement(const json& obj);
        player_movement(player_id_t p, const seat& f, const seat& t);
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
}

namespace std
{
    template<>
    struct hash<td::player>
    {
        typedef td::player argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const
        {
            result_type const h1 ( std::hash<std::string>()(s.name) );
            result_type const h2 ( std::hash<std::time_t>()(s.added_at) );
            return h1 ^ (h2 << 1);
        }
    };
}
