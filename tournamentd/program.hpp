#pragma once
#include <set>
#include <string>
#include <vector>
#include "socket.hpp"

class program
{
    inet_socket sv_sock;
    std::set<inet_socket> sockets;

public:
    // Create a program, given command line options
    explicit program(const std::vector<std::string>& cmdline);

    // Run one iteration of the program run loop
    bool run();
};