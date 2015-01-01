#include "json.hpp"
#include "types.hpp"
#include <unordered_map>

// ----- json speciailization

template <>
json& json::set_value(const char* name, const std::vector<td::blind_level>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("little_blind", value.little_blind);
        obj.set_value("big_blind", value.big_blind);
        obj.set_value("ante", value.ante);
        obj.set_value("duration_ms", value.duration);
        obj.set_value("break_duration_ms", value.break_duration);
        array.push_back(obj);
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
            array[i].get_value("little_blind", values[i].little_blind);
            array[i].get_value("big_blind", values[i].big_blind);
            array[i].get_value("ante", values[i].ante);
            array[i].get_value("duration_ms", values[i].duration);
            array[i].get_value("break_duration_ms", values[i].break_duration);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::chip>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("color", value.color);
        obj.set_value("denomination", value.denomination);
        obj.set_value("count_available", value.count_available);
        array.push_back(obj);
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
            array[i].get_value("color", values[i].color);
            array[i].get_value("denomination", values[i].denomination);
            array[i].get_value("count_available", values[i].count_available);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::funding_source>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("is_addon", value.is_addon);
        obj.set_value("forbid_after_blind_level", value.forbid_after_blind_level);
        obj.set_value("chips", value.chips);
        obj.set_value("cost", value.cost);
        obj.set_value("commission", value.commission);
        obj.set_value("equity", value.equity);
        array.push_back(obj);
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
            array[i].get_value("is_addon", values[i].is_addon);
            array[i].get_value("forbid_after_blind_level", values[i].forbid_after_blind_level);
            array[i].get_value("chips", values[i].chips);
            array[i].get_value("cost", values[i].cost);
            array[i].get_value("commission", values[i].commission);
            array[i].get_value("equity", values[i].equity);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::player>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("name", value.name);
        array.push_back(obj);
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
            array[i].get_value("name", values[i].name);
        }
        return true;
    }
    return false;
}

template <>
json& json::set_value(const char* name, const std::vector<td::seat>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("table_number", value.table_number);
        obj.set_value("seat_number", value.seat_number);
        array.push_back(obj);
    }
    return this->set_value(name, array);
}


template <>
json& json::set_value(const char* name, const std::unordered_map<td::player_id,td::seat>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("player_id", value.first);
        obj.set_value("table_number", value.second.table_number);
        obj.set_value("seat_number", value.second.seat_number);
        array.push_back(obj);
    }
    return this->set_value(name, array);
}
