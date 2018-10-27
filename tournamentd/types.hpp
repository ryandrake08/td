#pragma once
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
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
    enum class funding_source_type_t { buyin, rebuy, addon };

    // type of payout policy (automatic, forced, manual)
    enum class payout_policy_t { automatic, forced, manual };

    // type of rebalance policy (manual, automatic, shootout)
    enum class rebalance_policy_t { manual, automatic, shootout };

    // ante type
    enum class ante_type_t { none, traditional, bba };

    // final table policy
    enum class final_table_policy_t { fill, randomize };

    // attributes of an authorized client
    struct authorized_client
    {
        int code;
        std::string name;
        datetime added_at;

        authorized_client();
        authorized_client(int c, const std::string& name);
    };

    // attributes of a single blind level
    struct blind_level
    {
        std::string game_name;
        unsigned long little_blind;
        unsigned long big_blind;
        unsigned long ante;
        ante_type_t ante_type;
        long duration;
        long break_duration;
        std::string reason;

        blind_level();

        // equality
        bool operator==(const blind_level& other) const;
    };

    // attributes of a single chip denomination
    struct chip
    {
        std::string color;
        unsigned long denomination;
        unsigned long count_available;

        chip();

        // equality
        bool operator==(const chip& other) const;
    };

    // represents a monetary value
    // currency names use ISO 4217
    struct monetary_value
    {
        double amount;
        std::string currency;

        monetary_value();
        monetary_value(double amt, const std::string& curr);

        // equality
        bool operator==(const monetary_value& other) const;
    };

    // represents a monetary value without currency
    struct monetary_value_nocurrency
    {
        double amount;
        monetary_value_nocurrency();
        explicit monetary_value_nocurrency(double amt);
        
        // equality
        bool operator==(const monetary_value_nocurrency& other) const;
    };

    // attributes of each funding source (buy-in, addon, etc.)
    struct funding_source
    {
        std::string name;
        funding_source_type_t type;
        std::size_t forbid_after_blind_level;
        unsigned long chips;
        monetary_value cost;
        monetary_value commission;
        // equity currency must match configured payout_currency, so only amount is recorded here
        monetary_value_nocurrency equity;

        funding_source();

        // equality
        bool operator==(const funding_source& other) const;
    };

    // attributes of each player
    struct player
    {
        player_id_t player_id;
        std::string name;
        datetime added_at;

        player();

        // equality
        bool operator==(const player& other) const;
    };

    // attributes of a single physical seat at the tournament
    struct seat
    {
        std::size_t table_number;
        std::size_t seat_number;

        seat();
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
        player_movement(const player_id_t& p, const std::string& n, const seat& f, const seat& t);
    };

    // represents a quantity of chips distributed to each player
    struct player_chips
    {
        unsigned long denomination;
        unsigned long chips;

        player_chips();
        player_chips(unsigned long d, unsigned long c);
    };

    // represents a manually built payout structure
    struct manual_payout
    {
        size_t buyins_count;
        std::vector<td::monetary_value_nocurrency> payouts;

        manual_payout();
        manual_payout(size_t c, const std::vector<td::monetary_value_nocurrency>& p);
    };

    // represents a tournament result
    struct result
    {
        size_t place;
        std::string name;
        monetary_value payout;

        result();
        explicit result(size_t p, const std::string& n="");
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

    // automatic payout parameters
    struct automatic_payout_parameters
    {
        // configuration: automatic payouts: rough percentage of seats that get paid (0.0-1.0)
        double percent_seats_paid;

        // configuration: automatic payouts: round to whole numbers when calculating payouts?
        bool round_payouts;

        // configuration: automatic payouts: payout structure shape
        double payout_shape;

        // configuration: automatic payouts: how much to pay the bubble
        double pay_the_bubble;

        // configuration: automatic payouts: how much to set aside for each knockout
        double pay_knockouts;

        automatic_payout_parameters();
        automatic_payout_parameters(double percent_paid, bool round, double shape, double bubble, double knockouts);

        // equality
        bool operator==(const automatic_payout_parameters& other) const;
    };
}

// stream insertion
std::ostream& operator<<(std::ostream& os, const td::funding_source_type_t& value);
std::ostream& operator<<(std::ostream& os, const td::payout_policy_t& value);
std::ostream& operator<<(std::ostream& os, const td::rebalance_policy_t& value);
std::ostream& operator<<(std::ostream& os, const td::ante_type_t& value);
std::ostream& operator<<(std::ostream& os, const td::final_table_policy_t& value);
