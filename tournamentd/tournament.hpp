#pragma once
#include "json.hpp"
#include "gameclock.hpp"
#include "gamefunding.hpp"
#include "gameseating.hpp"
#include <string>
#include <vector>

class tournament
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

    // game state
    gameclock clock;
    gamefunding funding;
    gameseating seating;

public:
    // load configuration from JSON (object or file)
    void configure(const json& config);

    // accessors for game state
    gameclock& countdown_clock();
    gamefunding& funding_chart();
    gameseating& seating_chart();
};