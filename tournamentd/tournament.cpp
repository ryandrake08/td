#include "tournament.hpp"
#include "logger.hpp"

// load configuration from JSON (object or file)
void tournament::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    config.get_value("name", this->name);

    std::vector<json> array;

    if(config.get_value("players", array))
    {
        this->players.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            array[i].get_value("name", this->players[i].name);
        }
    }

    clock.configure(config);
    funding.configure(config);
    seating.configure(config);
}

// dump configuration to JSON
void tournament::dump_configuration(json& config) const
{
    config.set_value("name", this->name);

    std::vector<json> array;
    for(auto player : this->players)
    {
        array.push_back(json().set_value("name", player.name));
    }
    config.set_value("players", array);

    clock.dump_configuration(config);
    funding.dump_configuration(config);
    seating.dump_configuration(config);
}

// dump state to JSON
void tournament::dump_state(json& state) const
{
    clock.dump_state(state);
    funding.dump_state(state);
    seating.dump_state(state);
}

// accessors for game state
gameclock& tournament::countdown_clock()
{
    return this->clock;
}

gamefunding& tournament::funding_chart()
{
    return this->funding;
}

gameseating& tournament::seating_chart()
{
    return this->seating;
}
