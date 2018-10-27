#include "types.hpp"
#include <cassert>
#include <clocale>

// ----- initialization

// funding_source_type_t to stream
std::ostream& operator<<(std::ostream& os, const td::funding_source_type_t& value)
{
    switch(value)
    {
        case td::funding_source_type_t::buyin: os << "buyin"; break;
        case td::funding_source_type_t::rebuy: os << "rebuy"; break;
        case td::funding_source_type_t::addon: os << "addon"; break;
    }
    return os;
}

// funding_source_type_t to stream
std::ostream& operator<<(std::ostream& os, const td::payout_policy_t& value)
{
    switch(value)
    {
        case td::payout_policy_t::automatic: os << "automatic"; break;
        case td::payout_policy_t::forced: os << "forced"; break;
        case td::payout_policy_t::manual: os << "manual"; break;
    }
    return os;
}

// rebalance_policy_t to stream
std::ostream& operator<<(std::ostream& os, const td::rebalance_policy_t& value)
{
    switch(value)
    {
        case td::rebalance_policy_t::manual: os << "manual"; break;
        case td::rebalance_policy_t::automatic: os << "automatic"; break;
        case td::rebalance_policy_t::shootout: os << "shootout"; break;
    }
    return os;
}

// ante_type_t to stream
std::ostream& operator<<(std::ostream& os, const td::ante_type_t& value)
{
    switch(value)
    {
        case td::ante_type_t::none: os << "none"; break;
        case td::ante_type_t::traditional: os << "traditional"; break;
        case td::ante_type_t::bba: os << "bba"; break;
    }
    return os;
}

// rebalance_policy_t to stream
std::ostream& operator<<(std::ostream& os, const td::final_table_policy_t& value)
{
    switch(value)
    {
        case td::final_table_policy_t::fill: os << "fill"; break;
        case td::final_table_policy_t::randomize: os << "randomize"; break;
    }
    return os;
}

td::authorized_client::authorized_client() : code(0), added_at(datetime::now())
{
}

td::authorized_client::authorized_client(int c, const std::string& name) : code(c), name(name), added_at(datetime::now())
{
}

td::blind_level::blind_level() : little_blind(0), big_blind(0), ante(0), ante_type(td::ante_type_t::none), duration(0), break_duration(0)
{
}

bool td::blind_level::operator==(const blind_level& other) const
{
    return this->game_name == other.game_name &&
           this->little_blind == other.little_blind &&
           this->big_blind == other.big_blind &&
           this->ante == other.ante &&
           this->ante_type == other.ante_type &&
           this->duration == other.duration &&
           this->break_duration == other.break_duration &&
           this->reason == other.reason;
}

td::chip::chip() : denomination(0), count_available(0)
{
}

bool td::chip::operator==(const chip& other) const
{
    return this->color == other.color && this->denomination == other.denomination && this->count_available == other.count_available;
}

td::monetary_value::monetary_value() : amount(0.0)
{
    // use currency of default locale
    this->currency = std::localeconv()->int_curr_symbol;
}

td::monetary_value::monetary_value(double amt, const std::string& curr) : amount(amt), currency(curr)
{
}

bool td::monetary_value::operator==(const monetary_value& other) const
{
    return this->amount == other.amount && this->currency == other.currency;
}

td::monetary_value_nocurrency::monetary_value_nocurrency() : amount(0.0)
{
}

td::monetary_value_nocurrency::monetary_value_nocurrency(double amt) : amount(amt)
{
}

bool td::monetary_value_nocurrency::operator==(const monetary_value_nocurrency& other) const
{
    return this->amount == other.amount;
}

td::funding_source::funding_source() : type(td::funding_source_type_t::buyin), forbid_after_blind_level(std::numeric_limits<std::size_t>::max()), chips(0)
{
}

bool td::funding_source::operator==(const funding_source& other) const
{
    return this->name == other.name &&
           this->type == other.type &&
           this->forbid_after_blind_level == other.forbid_after_blind_level &&
           this->chips == other.chips &&
           this->cost == other.cost &&
           this->commission == other.commission &&
           this->equity == other.equity;
}

td::player::player() : added_at(datetime::now())
{
}

bool td::player::operator==(const player& other) const
{
    return this->player_id == other.player_id && this->name == other.name && this->added_at == other.added_at;
}

td::seat::seat() : table_number(0), seat_number(0)
{
}

td::seat::seat(std::size_t t, std::size_t s) : table_number(t), seat_number(s)
{
}

td::player_movement::player_movement()
{
}

td::player_movement::player_movement(const player_id_t& p, const std::string& n, const seat& f, const seat& t) : player_id(p), name(n), from_seat(f), to_seat(t)
{
}

td::player_chips::player_chips() : denomination(0), chips(0)
{
}

td::player_chips::player_chips(unsigned long d, unsigned long c) : denomination(d), chips(c)
{
}

td::manual_payout::manual_payout() : buyins_count(0)
{
}

td::manual_payout::manual_payout(size_t c, const std::vector<td::monetary_value_nocurrency>& p) : buyins_count(c), payouts(p)
{
}

td::result::result() : place(0)
{
}

td::result::result(size_t p, const std::string& n) : place(p), name(n)
{
}

td::seated_player::seated_player() : buyin(false), table_number(std::numeric_limits<std::size_t>::max()), seat_number(std::numeric_limits<std::size_t>::max())
{
}

td::seated_player::seated_player(const player_id_t& p, const std::string& n, bool b) : player_id(p), name(n), buyin(b), table_number(std::numeric_limits<std::size_t>::max()), seat_number(std::numeric_limits<std::size_t>::max())
{
}

td::seated_player::seated_player(const player_id_t& p, const std::string& n, bool b, std::size_t t, std::size_t s) : player_id(p), name(n), buyin(b), table_number(t), seat_number(s)
{
}

td::automatic_payout_parameters::automatic_payout_parameters() : percent_seats_paid(0.0), round_payouts(false), payout_shape(0.0), pay_the_bubble(0.0), pay_knockouts(0.0)
{
}

td::automatic_payout_parameters::automatic_payout_parameters(double percent_paid, bool round, double shape, double bubble, double knockouts) : percent_seats_paid(percent_paid), round_payouts(round), payout_shape(shape), pay_the_bubble(bubble), pay_knockouts(knockouts)
{
}

bool td::automatic_payout_parameters::operator==(const automatic_payout_parameters& other) const
{
    return this->percent_seats_paid == other.percent_seats_paid && this->round_payouts == other.round_payouts && this->payout_shape == other.payout_shape && this->pay_the_bubble == other.pay_the_bubble && this->pay_knockouts == other.pay_knockouts;
}

#include "json.hpp"

// ----- convert value from json

template <>
td::authorized_client json::value() const
{
    td::authorized_client ret;
    this->get_value("code", ret.code);
    this->get_value("name", ret.name);
    this->get_value("added_at", ret.added_at);
    return ret;
}

template <>
td::blind_level json::value() const
{
    td::blind_level ret;
    this->get_value("game_name", ret.game_name);
    this->get_value("little_blind", ret.little_blind);
    this->get_value("big_blind", ret.big_blind);
    this->get_value("ante", ret.ante);
    this->get_value("ante_type", reinterpret_cast<int&>(ret.ante_type));
    this->get_value("duration", ret.duration);
    this->get_value("break_duration", ret.break_duration);
    this->get_value("reason", ret.reason);
    return ret;
}

template <>
td::chip json::value() const
{
    td::chip ret;
    this->get_value("color", ret.color);
    this->get_value("denomination", ret.denomination);
    this->get_value("count_available", ret.count_available);
    return ret;
}

template <>
td::monetary_value json::value() const
{
    td::monetary_value ret;
    this->get_value("amount", ret.amount);
    this->get_value("currency", ret.currency);
    return ret;
}

template <>
td::monetary_value_nocurrency json::value() const
{
    td::monetary_value_nocurrency ret;
    this->get_value("amount", ret.amount);
    return ret;
}

template <>
td::funding_source json::value() const
{
    td::funding_source ret;
    this->get_value("name", ret.name);
    this->get_value("type", reinterpret_cast<int&>(ret.type));
    this->get_value("forbid_after_blind_level", ret.forbid_after_blind_level);
    this->get_value("chips", ret.chips);
    this->get_value("cost", ret.cost);
    this->get_value("commission", ret.commission);
    this->get_value("equity", ret.equity);
    return ret;
}

template <>
td::player json::value() const
{
    td::player ret;
    this->get_value("player_id", ret.player_id);
    this->get_value("name", ret.name);
    this->get_value("added_at", ret.added_at);
    return ret;
}

template <>
td::seat json::value() const
{
    td::seat ret;
    this->get_value("table_number", ret.table_number);
    this->get_value("seat_number", ret.seat_number);
    return ret;
}

template <>
td::manual_payout json::value() const
{
    td::manual_payout ret;
    this->get_value("buyins_count", ret.buyins_count);
    this->get_value("payouts", ret.payouts);
    return ret;
}

template <>
td::automatic_payout_parameters json::value() const
{
    td::automatic_payout_parameters ret;
    this->get_value("percent_seats_paid", ret.percent_seats_paid);
    this->get_value("round_payouts", ret.round_payouts);
    this->get_value("payout_shape", ret.payout_shape);
    this->get_value("pay_the_bubble", ret.pay_the_bubble);
    this->get_value("pay_knockouts", ret.pay_knockouts);
    return ret;
}

template <>
datetime json::value() const
{
    return datetime::from_gm(this->value<std::string>());
}

template <>
std::chrono::system_clock::time_point json::value() const
{
    std::chrono::milliseconds duration(this->value<long>());
    return std::chrono::system_clock::time_point(duration);
}

template <>
std::pair<td::player_id_t,td::seat> json::value() const
{
    std::pair<td::player_id_t,td::seat> ret;
    this->get_value("player_id", ret.first);
    this->get_value("table_number", ret.second.table_number);
    this->get_value("seat_number", ret.second.seat_number);
    return ret;
}

template <>
std::pair<std::string,double> json::value() const
{
    std::pair<std::string,double> ret;
    this->get_value("currency", ret.first);
    this->get_value("amount", ret.second);
    return ret;
}

// ----- construct json from object

template <>
json::json(const datetime& value) : json(value.gmtime())
{
}

template<>
json::json(const td::authorized_client& value) : json()
{
    this->set_value("code", value.code);
    if(!value.name.empty())
    {
        this->set_value("name", value.name);
    }
    if(value.added_at != datetime())
    {
        this->set_value("added_at", value.added_at);
    }
}

template<>
json::json(const td::blind_level& value) : json()
{
    if(!value.game_name.empty())
    {
        this->set_value("game_name", value.game_name);
    }
    if(value.little_blind > 0)
    {
        this->set_value("little_blind", value.little_blind);
    }
    if(value.big_blind > 0)
    {
        this->set_value("big_blind", value.big_blind);
    }
    if(value.ante > 0)
    {
        this->set_value("ante", value.ante);
    }

    this->set_value("ante_type", static_cast<int>(value.ante_type));

    if(value.duration> 0)
    {
        this->set_value("duration", value.duration);
    }
    if(value.break_duration > 0)
    {
        this->set_value("break_duration", value.break_duration);
    }
    if(!value.reason.empty())
    {
        this->set_value("reason", value.reason);
    }
}

template<>
json::json(const td::chip& value) : json()
{
    if(!value.color.empty())
    {
        this->set_value("color", value.color);
    }
    this->set_value("denomination", value.denomination);
    if(value.count_available > 0)
    {
        this->set_value("count_available", value.count_available);
    }
}

template<>
json::json(const td::monetary_value& value) : json()
{
    this->set_value("amount", value.amount);
    this->set_value("currency", value.currency);
}

template<>
json::json(const td::monetary_value_nocurrency& value) : json()
{
    this->set_value("amount", value.amount);
}

template<>
json::json(const td::funding_source& value) : json()
{
    this->set_value("name", value.name);
    this->set_value("type", static_cast<int>(value.type));
    if(value.forbid_after_blind_level != std::numeric_limits<std::size_t>::max())
    {
        this->set_value("forbid_after_blind_level", value.forbid_after_blind_level);
    }
    this->set_value("chips", value.chips);
    this->set_value("cost", json(value.cost));
    this->set_value("commission", value.commission);
    this->set_value("equity", value.equity);
}

template<>
json::json(const td::player& value) : json()
{
    this->set_value("player_id", value.player_id);
    this->set_value("name", value.name);
    this->set_value("added_at", value.added_at);
}

template<>
json::json(const td::seat& value) : json()
{
    this->set_value("table_number", value.table_number);
    this->set_value("seat_number", value.seat_number);
}

template<>
json::json(const td::player_movement& value) : json()
{
    this->set_value("player_id", value.player_id);
    this->set_value("name", value.name);
    this->set_value("from_table_number", value.from_seat.table_number);
    this->set_value("from_seat_number", value.from_seat.seat_number);
    this->set_value("to_table_number", value.to_seat.table_number);
    this->set_value("to_seat_number", value.to_seat.seat_number);
}

template<>
json::json(const td::player_chips& value) : json()
{
    this->set_value("denomination", value.denomination);
    this->set_value("chips", value.chips);
}

template<>
json::json(const std::pair<const td::player_id_t,td::seat>& value) : json()
{
    this->set_value("player_id", value.first);
    this->set_value("table_number", value.second.table_number);
    this->set_value("seat_number", value.second.seat_number);
}

template<>
json::json(const std::pair<const int,td::authorized_client>& value) : json(value.second)
{
    assert(value.first == value.second.code);
}

template<>
json::json(const std::pair<const td::player_id_t,td::player>& value) : json(value.second)
{
    assert(value.first == value.second.player_id);
}

template<>
json::json(const std::pair<const size_t,std::vector<td::monetary_value_nocurrency>>& value) : json()
{
    this->set_value("buyins_count", value.first);
    this->set_value("payouts", json(value.second.begin(), value.second.end()));
}

template<>
json::json(const td::result& value) : json()
{
    this->set_value("place", value.place);
    this->set_value("name", value.name);
    this->set_value("payout", value.payout);
}

template<>
json::json(const td::seated_player& value) : json()
{
    this->set_value("player_id", value.player_id);
    this->set_value("name", value.name);
    this->set_value("buyin", value.buyin);
    if(value.table_number != std::numeric_limits<std::size_t>::max())
    {
        this->set_value("table_number", value.table_number);
    }
    if(value.seat_number != std::numeric_limits<std::size_t>::max())
    {
        this->set_value("seat_number", value.seat_number);
    }
}

template<>
json::json(const std::pair<const std::string,double>& value) : json()
{
    this->set_value("currency", value.first);
    this->set_value("amount", value.second);
}

template<>
json::json(const td::automatic_payout_parameters& value) : json()
{
    this->set_value("percent_seats_paid", value.percent_seats_paid);
    this->set_value("round_payouts", value.round_payouts);
    this->set_value("payout_shape", value.payout_shape);
    this->set_value("pay_the_bubble", value.pay_the_bubble);
    this->set_value("pay_knockouts", value.pay_knockouts);
}
