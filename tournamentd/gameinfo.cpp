#include "gameinfo.hpp"
#include "logger.hpp"

// ----- game structure speciailization

template <>
json& json::set_value(const char* name, const std::vector<gameinfo::player>& values)
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
bool json::get_value(const char *name, std::vector<gameinfo::player>& values) const
{
    std::vector<json> array;
    if(this->get_value("funding_sources", array))
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

// load configuration from JSON (object or file)
void gameinfo::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    config.get_value("name", this->name);
    config.get_value("players", this->players);
}

// dump configuration to JSON
void gameinfo::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping tournament configuration\n";

    config.set_value("name", this->name);
    config.set_value("players", this->players);
}
