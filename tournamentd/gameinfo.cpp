#include "gameinfo.hpp"
#include "logger.hpp"

// load configuration from JSON (object or file)
void gameinfo::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    config.get_value("name", this->name);
    config.get_values("players", this->players);
}

// dump configuration to JSON
void gameinfo::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping tournament configuration\n";

    config.set_value("name", this->name);
    config.set_values("players", this->players);
}
