#pragma once
#include "json.hpp"
#include "types.hpp"
#include <unordered_map>

class gameinfo
{
    // configuration: list of all known players (playing or not)
    std::unordered_map<td::player_id,td::player> players;

public:
    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;
};