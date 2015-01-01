#pragma once
#include <chrono>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include "json.hpp"

namespace td
{
    class runtime_error : public std::runtime_error
    {
    public:
        explicit runtime_error(const char* what_arg) : std::runtime_error(what_arg) {}
    };

    // index into the player vector
    typedef std::size_t player_id;

    // index into the funding_source vector
    typedef std::size_t funding_source_id;

    // represents a point in time
    typedef std::chrono::system_clock::time_point tp;

    // represents a time duration
    typedef std::chrono::milliseconds ms;

    // used only for currency (buyin and payout)
    typedef std::size_t currency;

    // attributes of a single blind level
    struct blind_level : json::serializable
    {
        std::size_t little_blind;
        std::size_t big_blind;
        std::size_t ante;
        long duration;
        long break_duration;

        blind_level();
        void to_json(json& obj) const override;
        void from_json(const json& obj) override;
    };

    // attributes of a single chip denomination
    struct chip : json::serializable
    {
        std::string color;
        std::size_t denomination;
        std::size_t count_available;
        void to_json(json& obj) const override;
        void from_json(const json& obj) override;
    };

    // attributes of each funding source (buy-in, addon, etc.)
    struct funding_source : json::serializable
    {
        bool is_addon;
        std::size_t forbid_after_blind_level;
        std::size_t chips;
        currency cost;
        currency commission;
        currency equity;
        void to_json(json& obj) const override;
        void from_json(const json& obj) override;
    };

    // attributes of each player
    struct player : json::serializable
    {
        std::string name;

        void to_json(json& obj) const override;
        void from_json(const json& obj) override;
    };

    // attributes of a single physical seat at the tournament
    struct seat : json::serializable
    {
        std::size_t table_number;
        std::size_t seat_number;

        seat(std::size_t t, std::size_t s);
        void to_json(json& obj) const override;
        void from_json(const json& obj) override;
    };

    // represents a player's movement from one seat to another
    struct player_movement : json::serializable
    {
        player_id player;
        seat from_seat;
        seat to_seat;

        player_movement(player_id p, const seat& f, const seat& t);
        void to_json(json& obj) const override;
        void from_json(const json& obj) override;
    };
}
