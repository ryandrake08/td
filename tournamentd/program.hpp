#pragma once
#include <string>
#include <vector>
#include "server.hpp"
#include "tournament.hpp"

class program
{
    tournament game;
    server sv;

public:
    // Create a program, given command line options
    explicit program(const std::vector<std::string>& cmdline);

    // Run one iteration of the program run loop
    bool run();
};