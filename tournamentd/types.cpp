#include "types.hpp"

#include <utility>

// ----- construction

td::authorized_client::authorized_client() = default;

td::authorized_client::authorized_client(int c, std::string n) : code(c), name(std::move(n)), added_at(datetime::now())
{
}

td::blind_level::blind_level() = default;

bool td::blind_level::operator==(const td::blind_level& other) const
{
    return this->little_blind == other.little_blind &&
           this->big_blind == other.big_blind &&
           this->ante == other.ante &&
           this->ante_type == other.ante_type &&
           this->duration == other.duration &&
           this->break_duration == other.break_duration &&
           this->reason == other.reason;
}

td::chip::chip() = default;

bool td::chip::operator==(const td::chip& other) const
{
    return this->color == other.color && this->denomination == other.denomination && this->count_available == other.count_available;
}

td::table::table() = default;

bool td::table::operator==(const td::table& other) const
{
    return this->table_name == other.table_name;
}

td::monetary_value::monetary_value() = default;

td::monetary_value::monetary_value(double amt, std::string curr) : amount(amt), currency(std::move(curr))
{
}

bool td::monetary_value::operator==(const td::monetary_value& other) const
{
    return this->amount == other.amount && this->currency == other.currency;
}

td::monetary_value_nocurrency::monetary_value_nocurrency() = default;

td::monetary_value_nocurrency::monetary_value_nocurrency(double amt) : amount(amt)
{
}

bool td::monetary_value_nocurrency::operator==(const td::monetary_value_nocurrency& other) const
{
    return this->amount == other.amount;
}

td::funding_source::funding_source() = default;

bool td::funding_source::operator==(const td::funding_source& other) const
{
    return this->name == other.name &&
           this->type == other.type &&
           this->forbid_after_blind_level == other.forbid_after_blind_level &&
           this->chips == other.chips &&
           this->cost == other.cost &&
           this->commission == other.commission &&
           this->equity == other.equity;
}

td::player::player() = default;

bool td::player::operator==(const td::player& other) const
{
    return this->player_id == other.player_id && this->name == other.name && this->added_at == other.added_at;
}

td::seat::seat() = default;

td::seat::seat(std::size_t t, std::size_t s) : table_number(t), seat_number(s)
{
}

bool td::seat::operator==(const td::seat& other) const
{
    return this->seat_number == other.seat_number && this->table_number == other.table_number;
}

td::player_movement::player_movement() = default;

td::player_movement::player_movement(player_id_t p, std::string n, std::string ft, std::string fs, std::string tt, std::string ts) : player_id(std::move(p)), name(std::move(n)), from_table_name(std::move(ft)), from_seat_name(std::move(fs)), to_table_name(std::move(tt)), to_seat_name(std::move(ts))
{
}

td::player_chips::player_chips() = default;

td::player_chips::player_chips(unsigned long d, unsigned long c) : denomination(d), chips(c)
{
}

td::manual_payout::manual_payout() = default;

td::manual_payout::manual_payout(size_t c, const std::vector<td::monetary_value_nocurrency>& p) : buyins_count(c), payouts(p)
{
}

bool td::manual_payout::operator==(const td::manual_payout& other) const
{
    return this->buyins_count == other.buyins_count && this->payouts == other.payouts;
}

td::result::result() = default;

td::result::result(size_t p, std::string n) : place(p), name(std::move(n))
{
}

td::seated_player::seated_player(player_id_t p, bool b, std::string n) : player_id(std::move(p)), buyin(b), player_name(std::move(n))
{
}

td::seated_player::seated_player(player_id_t p, bool b, std::string n, std::string t, std::string s, const seat& pos) : player_id(std::move(p)), buyin(b), player_name(std::move(n)), table_name(std::move(t)), seat_name(std::move(s)), seat_position(pos)
{
}

td::seating_chart_entry::seating_chart_entry(std::string t, std::string s) : table_name(std::move(t)), seat_name(std::move(s))
{
}

td::seating_chart_entry::seating_chart_entry(std::string n, std::string t, std::string s) : player_name(std::move(n)), table_name(std::move(t)), seat_name(std::move(s))
{
}

td::automatic_payout_parameters::automatic_payout_parameters() = default;

td::automatic_payout_parameters::automatic_payout_parameters(double percent_paid, bool round, double shape, double bubble, double knockouts) : percent_seats_paid(percent_paid), round_payouts(round), payout_shape(shape), pay_the_bubble(bubble), pay_knockouts(knockouts)
{
}

bool td::automatic_payout_parameters::operator==(const td::automatic_payout_parameters& other) const
{
    return this->percent_seats_paid == other.percent_seats_paid && this->round_payouts == other.round_payouts && this->payout_shape == other.payout_shape && this->pay_the_bubble == other.pay_the_bubble && this->pay_knockouts == other.pay_knockouts;
}

#include "nlohmann/json.hpp"

// ----- convert types to and from json

void to_json(nlohmann::json& j, const datetime& p)
{
    j = p.gmtime();
}

void from_json(const nlohmann::json& j, datetime& p)
{
    p = datetime::from_gm(j);
}

namespace std
{
    namespace chrono
    {
        void to_json(nlohmann::json& j, const system_clock::time_point& p)
        {
            j = duration_cast<milliseconds>(p.time_since_epoch()).count();
        }

        void from_json(const nlohmann::json& j, system_clock::time_point& p)
        {
            p = system_clock::time_point(milliseconds(j));
        }
    }
};

void td::from_json(const nlohmann::json& j, td::authorized_client& p)
{
    p.code = j.value("code", 0);
    p.name = j.value("name", std::string());
    p.added_at = j.value("added_at", datetime::now());
}

void td::from_json(const nlohmann::json& j, td::blind_level& p)
{
    p.little_blind = j.value("little_blind", 0UL);
    p.big_blind = j.value("big_blind", 0UL);
    p.ante = j.value("ante", 0UL);
    p.ante_type = j.value("ante_type", td::ante_type_t::none);
    p.duration = j.value("duration", 0L);
    p.break_duration = j.value("break_duration", 0L);
    p.reason = j.value("reason", std::string());
}

void td::from_json(const nlohmann::json& j, td::chip& p)
{
    p.color = j.value("color", std::string());
    p.denomination = j.value("denomination", 0UL);
    p.count_available = j.value("count_available", 0UL);
}

void td::from_json(const nlohmann::json& j, td::table& p)
{
    p.table_name = j.value("table_name", std::string());
}

void td::from_json(const nlohmann::json& j, td::monetary_value& p)
{
    p.amount = j.value("amount", 0.0);
    p.currency = j.value("currency", std::string());
}

void td::from_json(const nlohmann::json& j, td::monetary_value_nocurrency& p)
{
    p.amount = j.value("amount", 0.0);
}

void td::from_json(const nlohmann::json& j, td::funding_source& p)
{
    p.name = j.value("name", std::string());
    p.type = j.value("type", td::funding_source_type_t::buyin);
    p.forbid_after_blind_level = j.value("forbid_after_blind_level", std::numeric_limits<std::size_t>::max());
    p.chips = j.value("chips", 0UL);
    p.cost = j.value("cost", td::monetary_value());
    p.commission = j.value("commission", td::monetary_value());
    p.equity = j.value("equity", td::monetary_value_nocurrency());
}

void td::from_json(const nlohmann::json& j, td::player& p)
{
    p.player_id = j.value("player_id", player_id_t());
    p.name = j.value("name", std::string());
    p.added_at = j.value("added_at", datetime::now());
}

void td::from_json(const nlohmann::json& j, td::seat& p)
{
    p.table_number = j.value("table_number", std::size_t { 0 });
    p.seat_number = j.value("seat_number", std::size_t { 0 });
}

void td::from_json(const nlohmann::json& j, td::manual_payout& p)
{
    p.buyins_count = j.value("buyins_count", std::size_t { 0 });
    p.payouts = j.value("payouts", std::vector<td::monetary_value_nocurrency>());
}

void td::from_json(const nlohmann::json& j, td::automatic_payout_parameters& p)
{
    p.percent_seats_paid = j.value("percent_seats_paid", 0.0);
    p.round_payouts = j.value("round_payouts", false);
    p.payout_shape = j.value("payout_shape", 0.0);
    p.pay_the_bubble = j.value("pay_the_bubble", 0.0);
    p.pay_knockouts = j.value("pay_knockouts", 0.0);
}

void td::to_json(nlohmann::json& j, const td::authorized_client& p)
{
    j = nlohmann::json {
        { "code", p.code }
    };

    if(!p.name.empty())
    {
        j["name"] = p.name;
    }
    if(p.added_at != datetime())
    {
        j["added_at"] = p.added_at;
    }
}

void td::to_json(nlohmann::json& j, const td::blind_level& p)
{
    j = nlohmann::json({});

    if(p.little_blind > 0)
    {
        j["little_blind"] = p.little_blind;
    }
    if(p.big_blind > 0)
    {
        j["big_blind"] = p.big_blind;
    }
    if(p.ante > 0)
    {
        j["ante"] = p.ante;
        j["ante_type"] = p.ante_type;
    }
    if(p.duration > 0)
    {
        j["duration"] = p.duration;
    }
    if(p.break_duration > 0)
    {
        j["break_duration"] = p.break_duration;
    }
    if(!p.reason.empty())
    {
        j["reason"] = p.reason;
    }
}

void td::to_json(nlohmann::json& j, const td::chip& p)
{
    j = nlohmann::json {
        { "denomination", p.denomination }
    };

    if(!p.color.empty())
    {
        j["color"] = p.color;
    }
    if(p.count_available > 0)
    {
        j["count_available"] = p.count_available;
    }
}

void td::to_json(nlohmann::json& j, const td::table& p)
{
    j = nlohmann::json({});

    if(!p.table_name.empty())
    {
        j["table_name"] = p.table_name;
    }
}

void td::to_json(nlohmann::json& j, const td::monetary_value& p)
{
    j = nlohmann::json {
        { "amount", p.amount }
    };

    if(!p.currency.empty())
    {
        j["currency"] = p.currency;
    }
}

void td::to_json(nlohmann::json& j, const td::monetary_value_nocurrency& p)
{
    j = nlohmann::json {
        { "amount", p.amount }
    };
}

void td::to_json(nlohmann::json& j, const td::funding_source& p)
{
    j = nlohmann::json {
        { "name", p.name },
        { "type", p.type },
        { "chips", p.chips },
        { "cost", p.cost },
        { "commission", p.commission },
        { "equity", p.equity }
    };

    if(p.forbid_after_blind_level != std::numeric_limits<std::size_t>::max())
    {
        j["forbid_after_blind_level"] = p.forbid_after_blind_level;
    }
}

void td::to_json(nlohmann::json& j, const td::player& p)
{
    j = nlohmann::json {
        { "player_id", p.player_id },
        { "name", p.name },
        { "added_at", p.added_at }
    };
}

void td::to_json(nlohmann::json& j, const td::seat& p)
{
    j = nlohmann::json {
        { "table_number", p.table_number },
        { "seat_number", p.seat_number }
    };
}

void td::to_json(nlohmann::json& j, const td::manual_payout& p)
{
    j = nlohmann::json {
        { "buyins_count", p.buyins_count },
        { "payouts", p.payouts }
    };
}

void td::to_json(nlohmann::json& j, const td::automatic_payout_parameters& p)
{
    j = nlohmann::json {
        { "percent_seats_paid", p.percent_seats_paid },
        { "round_payouts", p.round_payouts },
        { "payout_shape", p.payout_shape },
        { "pay_the_bubble", p.pay_the_bubble },
        { "pay_knockouts", p.pay_knockouts }
    };
}

void td::to_json(nlohmann::json& j, const td::player_movement& p)
{
    j = nlohmann::json {
        { "player_id", p.player_id },
        { "name", p.name },
        { "from_table_name", p.from_table_name },
        { "from_seat_name", p.from_seat_name },
        { "to_table_name", p.to_table_name },
        { "to_seat_name", p.to_seat_name }
    };
}

void td::to_json(nlohmann::json& j, const td::player_chips& p)
{
    j = nlohmann::json {
        { "denomination", p.denomination },
        { "chips", p.chips }
    };
}

void td::to_json(nlohmann::json& j, const td::result& p)
{
    j = nlohmann::json {
        { "place", p.place },
        { "name", p.name },
        { "payout", p.payout }
    };
}

void td::to_json(nlohmann::json& j, const td::seated_player& p)
{
    if(p.seat_name.empty())
    {
        j = nlohmann::json {
            { "player_id", p.player_id },
            { "buyin", p.buyin },
            { "player_name", p.player_name },
            { "seat_position", p.seat_position }
        };
    }
    else
    {
        j = nlohmann::json {
            { "player_id", p.player_id },
            { "buyin", p.buyin },
            { "player_name", p.player_name },
            { "table_name", p.table_name },
            { "seat_name", p.seat_name },
            { "seat_position", p.seat_position }
        };
    }
}

void td::to_json(nlohmann::json& j, const td::seating_chart_entry& p)
{
    if(p.player_name.empty())
    {
        j = nlohmann::json {
            { "table_name", p.table_name },
            { "seat_name", p.seat_name }
        };
    }
    else
    {
        j = nlohmann::json {
            { "player_name", p.player_name },
            { "table_name", p.table_name },
            { "seat_name", p.seat_name }
        };
    }
}

// ----- ostream insertion

// funding_source_type_t to stream
std::ostream& operator<<(std::ostream& os, const td::funding_source_type_t& value)
{
    switch(value)
    {
    case td::funding_source_type_t::buyin:
        os << "buyin";
        break;
    case td::funding_source_type_t::rebuy:
        os << "rebuy";
        break;
    case td::funding_source_type_t::addon:
        os << "addon";
        break;
    }
    return os;
}

// funding_source_type_t to stream
std::ostream& operator<<(std::ostream& os, const td::payout_policy_t& value)
{
    switch(value)
    {
    case td::payout_policy_t::automatic:
        os << "automatic";
        break;
    case td::payout_policy_t::forced:
        os << "forced";
        break;
    case td::payout_policy_t::manual:
        os << "manual";
        break;
    }
    return os;
}

// rebalance_policy_t to stream
std::ostream& operator<<(std::ostream& os, const td::rebalance_policy_t& value)
{
    switch(value)
    {
    case td::rebalance_policy_t::manual:
        os << "manual";
        break;
    case td::rebalance_policy_t::automatic:
        os << "automatic";
        break;
    case td::rebalance_policy_t::shootout:
        os << "shootout";
        break;
    }
    return os;
}

// ante_type_t to stream
std::ostream& operator<<(std::ostream& os, const td::ante_type_t& value)
{
    switch(value)
    {
    case td::ante_type_t::none:
        os << "none";
        break;
    case td::ante_type_t::traditional:
        os << "traditional";
        break;
    case td::ante_type_t::bba:
        os << "bba";
        break;
    }
    return os;
}

// rebalance_policy_t to stream
std::ostream& operator<<(std::ostream& os, const td::final_table_policy_t& value)
{
    switch(value)
    {
    case td::final_table_policy_t::fill:
        os << "fill";
        break;
    case td::final_table_policy_t::randomize:
        os << "randomize";
        break;
    }
    return os;
}

// blind level to string
std::ostream& operator<<(std::ostream& os, const td::blind_level& level)
{
    os << level.little_blind << '/' << level.big_blind;
    if(level.ante_type != td::ante_type_t::none)
    {
        // TODO: i18n
        os << "\nAnte: " << level.ante;
    }
    return os;
}
