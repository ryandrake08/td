#include "gameinfo.hpp"
#include "logger.hpp"

// load configuration from JSON (object or file)
void gameinfo::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    config.get_value("name", this->name);

    // special handling for players, read into vector, then convert to map
    std::vector<td::player> players_vector;
    if(config.get_values("players", players_vector))
    {
        this->players.clear();
        for(auto player : players_vector)
        {
            this->players.emplace(std::hash<td::player>()(player), player);
        }
    }
}

// dump configuration to JSON
void gameinfo::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping tournament configuration\n";

    config.set_value("name", this->name);
    config.set_values("players", this->players);
}
