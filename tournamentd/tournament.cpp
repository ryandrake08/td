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
