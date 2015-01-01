#pragma once
#include "json.hpp"
#include "types.hpp"
#include <string>
#include <vector>

class gameinfo
{
    // configuration: game name
    std::string name;

    // configuration: list of all known players (playing or not)
    std::vector<td::player> players;

public:
    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;
};