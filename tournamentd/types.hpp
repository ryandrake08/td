#pragma once
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#include "datetime.hpp"
#include "nlohmann/json_fwd.hpp"

// convert datetime to and from json
void to_json(nlohmann::json& j, const datetime& p);
void from_json(const nlohmann::json& j, datetime& p);

// convert time_point to and from json
namespace std
{
    namespace chrono
    {
        void to_json(nlohmann::json& j, const system_clock::time_point& p);
        void from_json(const nlohmann::json& j, system_clock::time_point& p);
    };
};

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
        authorized_client(int c, const std::string& n);
    };
    void to_json(nlohmann::json& j, const td::authorized_client& p);
    void from_json(const nlohmann::json& j, td::authorized_client& p);

    // attributes of a single blind level
    struct blind_level
    {
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
    void to_json(nlohmann::json& j, const td::blind_level& p);
    void from_json(const nlohmann::json& j, td::blind_level& p);

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
    void to_json(nlohmann::json& j, const td::chip& p);
    void from_json(const nlohmann::json& j, td::chip& p);

    // attributes of a single named table
    struct table
    {
        std::string table_name;

        table();

        // equality
        bool operator==(const table& other) const;
    };
    void to_json(nlohmann::json& j, const td::table& p);
    void from_json(const nlohmann::json& j, td::table& p);

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
    void to_json(nlohmann::json& j, const td::monetary_value& p);
    void from_json(const nlohmann::json& j, td::monetary_value& p);

    // represents a monetary value without currency
    struct monetary_value_nocurrency
    {
        double amount;
        monetary_value_nocurrency();
        explicit monetary_value_nocurrency(double amt);

        // equality
        bool operator==(const monetary_value_nocurrency& other) const;
    };
    void to_json(nlohmann::json& j, const td::monetary_value_nocurrency& p);
    void from_json(const nlohmann::json& j, td::monetary_value_nocurrency& p);

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
    void to_json(nlohmann::json& j, const td::funding_source& p);
    void from_json(const nlohmann::json& j, td::funding_source& p);

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
    void to_json(nlohmann::json& j, const td::player& p);
    void from_json(const nlohmann::json& j, td::player& p);

    // attributes of a single physical seat at the tournament
    struct seat
    {
        std::size_t table_number;
        std::size_t seat_number;

        seat();
        seat(std::size_t t, std::size_t s);

        // equality
        bool operator==(const seat& other) const;
    };
    void to_json(nlohmann::json& j, const td::seat& p);
    void from_json(const nlohmann::json& j, td::seat& p);

    // represents a player's movement from one seat to another
    struct player_movement
    {
        player_id_t player_id;
        std::string name;
        std::string from_table_name;
        std::string from_seat_name;
        std::string to_table_name;
        std::string to_seat_name;

        player_movement();
        player_movement(const player_id_t& p, const std::string& n, const std::string& ft, const std::string& fs, const std::string& tt=std::string(), const std::string& ts=std::string());
    };
    void to_json(nlohmann::json& j, const td::player_movement& p);

    // represents a quantity of chips distributed to each player
    struct player_chips
    {
        unsigned long denomination;
        unsigned long chips;

        player_chips();
        player_chips(unsigned long d, unsigned long c);
    };
    void to_json(nlohmann::json& j, const td::player_chips& p);

    // represents a manually built payout structure
    struct manual_payout
    {
        size_t buyins_count;
        std::vector<td::monetary_value_nocurrency> payouts;

        manual_payout();
        manual_payout(size_t c, const std::vector<td::monetary_value_nocurrency>& p);

        // equality
        bool operator==(const manual_payout& other) const;
    };
    void to_json(nlohmann::json& j, const td::manual_payout& p); // TODO: Need this?
    void from_json(const nlohmann::json& j, td::manual_payout& p);

    // represents a tournament result
    struct result
    {
        size_t place;
        std::string name;
        monetary_value_nocurrency payout;

        result();
        explicit result(size_t p, const std::string& n="");
    };
    void to_json(nlohmann::json& j, const td::result& p);

    // represents a player with additional buyin/seat info
    struct seated_player
    {
        player_id_t player_id;
        bool buyin;
        std::string player_name;
        std::string table_name;
        std::string seat_name;

        // unseated player
        seated_player(const player_id_t& p, bool b, const std::string& n);
        // seated player
        seated_player(const player_id_t& p, bool b, const std::string& n, const std::string& t, const std::string& s);
    };
    void to_json(nlohmann::json& j, const td::seated_player& p);

    // information needed to display the seating chart
    struct seating_chart_entry
    {
        std::string player_name;
        std::string table_name;
        std::string seat_name;

        // empty seat
        seating_chart_entry(const std::string& t, const std::string& s);
        // seat with player
        seating_chart_entry(const std::string& n, const std::string& t, const std::string& s);
    };
    void to_json(nlohmann::json& j, const td::seating_chart_entry& p);

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
    void to_json(nlohmann::json& j, const td::automatic_payout_parameters& p); // TODO: Needed?
    void from_json(const nlohmann::json& j, td::automatic_payout_parameters& p);
}

// stream insertion
std::ostream& operator<<(std::ostream& os, const td::funding_source_type_t& value);
std::ostream& operator<<(std::ostream& os, const td::payout_policy_t& value);
std::ostream& operator<<(std::ostream& os, const td::rebalance_policy_t& value);
std::ostream& operator<<(std::ostream& os, const td::ante_type_t& value);
std::ostream& operator<<(std::ostream& os, const td::final_table_policy_t& value);
std::ostream& operator<<(std::ostream& os, const td::blind_level& level);
