#include "json.hpp"
#include "types.hpp"
#include <unordered_map>

// ----- initialization

td::blind_level::blind_level() : little_blind(0), big_blind(0), ante(0), duration(0), break_duration(0)
{
}

td::seat::seat(std::size_t t, std::size_t s) : table_number(t), seat_number(s)
{
}

td::player_movement::player_movement(player_id p, const seat& f, const seat& t) : player(p), from_seat(f), to_seat(t)
{
}

// ----- serialization

void td::blind_level::to_json(json& obj) const
{
    obj.set_value("little_blind", this->little_blind);
    obj.set_value("big_blind", this->big_blind);
    obj.set_value("ante", this->ante);
    obj.set_value("duration_ms", this->duration);
    obj.set_value("break_duration_ms", this->break_duration);
}

void td::blind_level::from_json(const json& obj)
{
    obj.get_value("little_blind", this->little_blind);
    obj.get_value("big_blind", this->big_blind);
    obj.get_value("ante", this->ante);
    obj.get_value("duration_ms", this->duration);
    obj.get_value("break_duration_ms", this->break_duration);
}

void td::chip::to_json(json& obj) const
{
    obj.set_value("color", this->color);
    obj.set_value("denomination", this->denomination);
    obj.set_value("count_available", this->count_available);
}

void td::chip::from_json(const json& obj)
{
    obj.get_value("color", this->color);
    obj.get_value("denomination", this->denomination);
    obj.get_value("count_available", this->count_available);
}

void td::funding_source::to_json(json& obj) const
{
    obj.set_value("is_addon", this->is_addon);
    obj.set_value("forbid_after_blind_level", this->forbid_after_blind_level);
    obj.set_value("chips", this->chips);
    obj.set_value("cost", this->cost);
    obj.set_value("commission", this->commission);
    obj.set_value("equity", this->equity);
}

void td::funding_source::from_json(const json& obj)
{
    obj.get_value("is_addon", this->is_addon);
    obj.get_value("forbid_after_blind_level", this->forbid_after_blind_level);
    obj.get_value("chips", this->chips);
    obj.get_value("cost", this->cost);
    obj.get_value("commission", this->commission);
    obj.get_value("equity", this->equity);
}

void td::player::to_json(json& obj) const
{
    obj.set_value("name", this->name);
}

void td::player::from_json(const json& obj)
{
    obj.get_value("name", this->name);
}

void td::seat::to_json(json& obj) const
{
    obj.set_value("table_number", this->table_number);
    obj.set_value("seat_number", this->seat_number);
}

void td::seat::from_json(const json& obj)
{
    obj.get_value("table_number", this->table_number);
    obj.get_value("seat_number", this->seat_number);
}

void td::player_movement::to_json(json& obj) const
{
    obj.set_value("player_id", this->player);
    obj.set_value("from_table_number", this->from_seat.table_number);
    obj.set_value("from_seat_number", this->from_seat.seat_number);
    obj.set_value("to_table_number", this->to_seat.table_number);
    obj.set_value("to_seat_number", this->to_seat.seat_number);
}

void td::player_movement::from_json(const json& obj)
{
    obj.get_value("player_id", this->player);
    obj.get_value("from_table_number", this->from_seat.table_number);
    obj.get_value("from_seat_number", this->from_seat.seat_number);
    obj.get_value("to_table_number", this->to_seat.table_number);
    obj.get_value("to_seat_number", this->to_seat.seat_number);
}

// ----- json speciailization

template <>
json& json::set_value(const char* name, const std::vector<td::blind_level>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        array.push_back(json(value));
    }
    return this->set_value(name, array);
}


template <>
bool json::get_value(const char *name, std::vector<td::blind_level>& values) const
{
    std::vector<json> array;
    if(this->get_value(name, array))
    {
        values.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            values[i].from_json(array[i]);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::chip>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        array.push_back(json(value));
    }
    return this->set_value(name, array);
}

template <>
bool json::get_value(const char *name, std::vector<td::chip>& values) const
{
    std::vector<json> array;
    if(this->get_value(name, array))
    {
        values.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            values[i].from_json(array[i]);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::funding_source>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        array.push_back(json(value));
    }
    return this->set_value(name, array);
}

template <>
bool json::get_value(const char *name, std::vector<td::funding_source>& values) const
{
    std::vector<json> array;
    if(this->get_value(name, array))
    {
        values.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            values[i].from_json(array[i]);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::player>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        array.push_back(json(value));
    }
    return this->set_value(name, array);
}

template <>
bool json::get_value(const char *name, std::vector<td::player>& values) const
{
    std::vector<json> array;
    if(this->get_value(name, array))
    {
        values.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            values[i].from_json(array[i]);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::seat>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        array.push_back(json(value));
    }
    return this->set_value(name, array);
}

template <>
json& json::set_value(const char* name, const std::unordered_map<td::player_id,td::seat>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        json obj;
        obj.set_value("player_id", value.first);
        obj.set_value("table_number", value.second.table_number);
        obj.set_value("seat_number", value.second.seat_number);
        array.push_back(obj);
    }
    return this->set_value(name, array);
}

template <typename T>
json jsonify(const T& value)
{
    return json(value);
}

template <>
json& json::set_value(const char* name, const std::vector<td::player_movement>& values)
{
    std::vector<json> array;
    array.reserve(values.size());
    for(const auto& value : values)
    {
        array.push_back(json(value));
    }
    return this->set_value(name, array);
}
