#include "types.hpp"
#include <utility>

// ----- initialization

td::blind_level::blind_level() : little_blind(0), big_blind(0), ante(0), duration(0), break_duration(0)
{
}

td::chip::chip() : denomination(0), count_available(0)
{
}

td::funding_source::funding_source() : is_addon(false), forbid_after_blind_level(std::numeric_limits<std::size_t>::max()), chips(0), cost(0.0), commission(0.0), equity(0.0)
{
}

td::player::player() : added_at(datetime::now())
{
}

td::seat::seat() : table_number(0), seat_number(0)
{
}

td::seat::seat(std::size_t t, std::size_t s) : table_number(t), seat_number(s)
{
}

td::player_movement::player_movement() : player(0)
{
}

td::player_movement::player_movement(player_id p, const seat& f, const seat& t) : player(p), from_seat(f), to_seat(t)
{
}

td::player_chips::player_chips() : denomination(0), chips(0)
{
}

td::player_chips::player_chips(unsigned long d, unsigned long c) : denomination(d), chips(c)
{
}

// ----- construct from json

td::blind_level::blind_level(const json& obj) : blind_level()
{
    obj.get_value("little_blind", this->little_blind);
    obj.get_value("big_blind", this->big_blind);
    obj.get_value("ante", this->ante);
    obj.get_value("duration", this->duration);
    obj.get_value("break_duration", this->break_duration);
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
    obj.get_value("is_addon", this->is_addon);
    obj.get_value("forbid_after_blind_level", this->forbid_after_blind_level);
    obj.get_value("chips", this->chips);
    obj.get_value("cost", this->cost);
    obj.get_value("commission", this->commission);
    obj.get_value("equity", this->equity);
}

td::player::player(const json& obj) : player()
{
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
    obj.get_value("player_id", this->player);
    obj.get_value("from_table_number", this->from_seat.table_number);
    obj.get_value("from_seat_number", this->from_seat.seat_number);
    obj.get_value("to_table_number", this->to_seat.table_number);
    obj.get_value("to_seat_number", this->to_seat.seat_number);
}

// ----- construct json from object

template<>
json::json(const td::blind_level& value) : json()
{
    this->set_value("little_blind", value.little_blind);
    this->set_value("big_blind", value.big_blind);
    this->set_value("ante", value.ante);
    this->set_value("duration", value.duration);
    this->set_value("break_duration", value.break_duration);
}

template<>
json::json(const td::chip& value) : json()
{
    this->set_value("color", value.color);
    this->set_value("denomination", value.denomination);
    this->set_value("count_available", value.count_available);
}

template<>
json::json(const td::funding_source& value) : json()
{
    this->set_value("name", value.name);
    this->set_value("is_addon", value.is_addon);
    if(value.forbid_after_blind_level != std::numeric_limits<std::size_t>::max())
    {
        this->set_value("forbid_after_blind_level", value.forbid_after_blind_level);
    }
    this->set_value("chips", value.chips);
    this->set_value("cost", value.cost);
    this->set_value("commission", value.commission);
    this->set_value("equity", value.equity);
}

template<>
json::json(const td::player& value) : json()
{
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
    this->set_value("player_id", value.player);
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
json::json(const std::pair<const td::player_id,td::seat>& value) : json()
{
    this->set_value("player_id", value.first);
    this->set_value("table_number", value.second.table_number);
    this->set_value("seat_number", value.second.seat_number);
}

template<>
json::json(const std::pair<const td::player_id,td::player>& value) : json()
{
    this->set_value("player_id", value.first);
    this->set_value("name", value.second.name);
    this->set_value("added_at", value.second.added_at);
}
