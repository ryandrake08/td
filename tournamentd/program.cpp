#include "program.hpp"
#include "logger.hpp"
#include <iostream>
#include <system_error>

program::program(const std::vector<std::string>& cmdline) : sv(25600)
{
}

bool program::run()
{
    auto handle_new_client = [](std::iostream& client)
    {
        static const char* name = "tournamentd";
        static const char* version = "0.0.9";

        // greet client
        json greeting;
        greeting.set_value("server_name", name);
        greeting.set_value("server_version", version);
        greeting.set_value("ready", true);
        client << greeting;
    };

    auto handle_client = [this](std::iostream& client)
    {
        std::string cmd;
        std::getline(client, cmd);

        json out;
        this->game.dump_configuration(out);
        this->game.dump_state(out);
        client << out;
    };

    this->game.countdown_clock().update_remaining();
    return this->sv.poll(handle_new_client, handle_client, 50000);
}