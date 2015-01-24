#pragma once
#include <string>
#include <vector>
#include "tournament.hpp"

class program
{
    tournament tourney;

public:
    // Create a program, given command line options
    explicit program(const std::vector<std::string>& cmdline);

    // Run one iteration of the program run loop, returns true to exit
    bool run();

    // Handle SIGUSR2 user event, returning true, returns true to exit
    bool sigusr2();
};