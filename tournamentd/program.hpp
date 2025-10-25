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

    // Non-copyable, non-movable (manages unique resources)
    program(const program&) = delete;
    program& operator=(const program&) = delete;
    program(program&&) = delete;
    program& operator=(program&&) = delete;

    // Run one iteration of the program run loop, returns true to exit
    bool run();

    // Handle SIGUSR2 user event, returns true to exit
    bool sigusr2();
};
