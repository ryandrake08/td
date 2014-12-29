#pragma once
#include "json.hpp"
#include <string>
#include <vector>

class gameinfo
{
public:
    struct player
    {
        std::string name;
    };

private:
    // configuration: game name
    std::string name;

    // configuration: list of all known players (playing or not)
    std::vector<player> players;

public:
    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;
};