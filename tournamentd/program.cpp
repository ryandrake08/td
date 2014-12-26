#include "program.hpp"
#include "logger.hpp"
#include <iostream>
#include <system_error>

#include "socketstream.hpp"

program::program(const std::vector<std::string>& cmdline) : sv(25600)
{
}

bool program::run()
{
    sv.poll(50000);
    game.countdown_clock().update_remaining();
    return false;
}