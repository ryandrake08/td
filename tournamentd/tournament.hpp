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

    // dump configuration to JSON
    void dump_configuration(json& config) const;

    // dump state to JSON
    void dump_state(json& state) const;

    // accessors for game state
    gameclock& countdown_clock();
    const gameclock& countdown_clock() const;

    gamefunding& funding_chart();
    const gamefunding& funding_chart() const;

    gameseating& seating_chart();
    const gameseating& seating_chart() const;
};