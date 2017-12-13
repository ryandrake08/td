#include "types.hpp"
#include "shared_instance.hpp"
#include <random>
#include <utility>
#include <cassert>

static std::string random_player_id()
{
    // get randomization engine
    auto engine(get_shared_instance<std::default_random_engine>());
    auto distribution(std::uniform_int_distribution<int>(0, std::numeric_limits<int>::max()));
    auto numeric_id(distribution(*engine));
    return std::to_string(numeric_id);
}

// ----- initialization

td::authorized_client::authorized_client(int c) : code(c), added_at(datetime::now())
{
}

td::blind_level::blind_level() : little_blind(0), big_blind(0), ante(0), duration(0), break_duration(0)
{
}

td::chip::chip() : denomination(0), count_available(0)
{
}

td::funding_source::funding_source() : type(td::buyin), forbid_after_blind_level(std::numeric_limits<std::size_t>::max()), chips(0), cost(0.0), commission(0.0), equity(0.0)
{
}

td::player::player() : player_id(random_player_id()), added_at(datetime::now())
{
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

td::manual_payout::manual_payout(size_t c, const std::vector<double>& p) : buyins_count(c), payouts(p)
{
}

td::result::result() : place(0), payout(0.0)
{
}

td::result::result(size_t p, const std::string& n) : place(p), name(n), payout(0.0)
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

// ----- construct from json

td::authorized_client::authorized_client(const json& obj)
{
    obj.get_value("code", this->code);
    obj.get_value("name", this->name);
    obj.get_value("added_at", this->added_at);
}

td::blind_level::blind_level(const json& obj) : blind_level()
{
    obj.get_value("game_name", this->game_name);
    obj.get_value("little_blind", this->little_blind);
    obj.get_value("big_blind", this->big_blind);
    obj.get_value("ante", this->ante);
    obj.get_value("duration", this->duration);
    obj.get_value("break_duration", this->break_duration);
    obj.get_value("reason", this->reason);
}

td::chip::chip(const json& obj) : chip()
{
    obj.get_value("color", this->color);
    obj.get_value("denomination", this->denomination);
    obj.get_value("count_available", this->count_available);
}

td::funding_source::funding_source(const json& obj) : funding_source()
{
    obj.get_value("name", this->name);
    obj.get_value("type", reinterpret_cast<int&>(this->type));
    obj.get_value("forbid_after_blind_level", this->forbid_after_blind_level);
    obj.get_value("chips", this->chips);
    obj.get_value("cost", this->cost);
    obj.get_value("commission", this->commission);
    obj.get_value("equity", this->equity);
    obj.get_value("cost_currency", this->cost_currency);
    obj.get_value("commission_currency", this->commission_currency);
    obj.get_value("equity_currency", this->equity_currency);
}

td::player::player(const json& obj) : player()
{
    obj.get_value("player_id", this->player_id);
    obj.get_value("name", this->name);
    obj.get_value("added_at", this->added_at);
}

td::seat::seat(const json& obj) : seat()
{
    obj.get_value("table_number", this->table_number);
    obj.get_value("seat_number", this->seat_number);
}

td::player_movement::player_movement(const json& obj) : player_movement()
{
    obj.get_value("player_id", this->player_id);
    obj.get_value("name", this->name);
    obj.get_value("from_table_number", this->from_seat.table_number);
    obj.get_value("from_seat_number", this->from_seat.seat_number);
    obj.get_value("to_table_number", this->to_seat.table_number);
    obj.get_value("to_seat_number", this->to_seat.seat_number);
}

td::manual_payout::manual_payout(const json& obj) : manual_payout()
{
    obj.get_value("buyins_count", this->buyins_count);
    obj.get_values("payouts", this->payouts);
}

// ----- return object from json

template <>
datetime json::value() const
{
    return datetime::from_gm(this->value<std::string>());
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
json::json(const td::funding_source& value) : json()
{
    this->set_value("name", value.name);
    this->set_value("type", static_cast<int>(value.type));
    if(value.forbid_after_blind_level != std::numeric_limits<std::size_t>::max())
    {
        this->set_value("forbid_after_blind_level", value.forbid_after_blind_level);
    }
    this->set_value("chips", value.chips);
    this->set_value("cost", value.cost);
    this->set_value("commission", value.commission);
    this->set_value("equity", value.equity);
    this->set_value("cost_currency", value.cost_currency);
    this->set_value("commission_currency", value.commission_currency);
    this->set_value("equity_currency", value.equity_currency);
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
json::json(const std::pair<const size_t,std::vector<double>>& value) : json()
{
    this->set_value("buyins_count", value.first);
    this->set_value("payouts", json(value.second.begin(), value.second.end()));
}

template<>
json::json(const td::result& value) : json()
{
    this->set_value("place", value.place);
    this->set_value("name", value.name);
    if(value.payout != 0.0)
    {
        this->set_value("payout", value.payout);
        this->set_value("payout_currency", value.payout_currency);
    }
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
