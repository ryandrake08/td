#pragma once
#include <memory>
#include <string>
#include <vector>

class program
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    // Create a program, given command line options
    explicit program(const std::vector<std::string>& cmdline);

    // Destructor
    ~program();

    // Run one iteration of the program run loop, returns true to exit
    bool run();

    // Handle SIGUSR2 user event, returns true to exit
    bool sigusr2();
};
