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

    struct chip
    {
        std::string color;
        std::size_t denomination;
        std::size_t count_available;
    };

private:
    // configuration: game name
    std::string name;

    // configuration: list of all known players (playing or not)
    std::vector<player> players;

    // configuration: description of each chip (for display)
    std::vector<chip> chips;

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