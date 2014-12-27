#include "program.hpp"
#include "logger.hpp"
#include <iostream>
#include <system_error>

program::program(const std::vector<std::string>& cmdline) : sv(25600)
{
}

static void handle_new_client(std::ostream& client, const tournament& game)
{
    static const char* name = "tournamentd";
    static const char* version = "0.0.9";

    // greet client
    json out;
    out.set_value("server_name", name);
    out.set_value("server_version", version);
    game.dump_configuration(out);
    game.dump_state(out);
    out.set_value("ready", true);
    client << out;
}

static void handle_client_input(std::iostream& client, tournament& game)
{
    std::string cmd;
    std::getline(client, cmd);

    json out;
    client << out;
}

bool program::run()
{
    // update the clock, and report to clients if anything changed
    if(this->game.countdown_clock().update_remaining())
    {
        // get clock state
        json out;
        this->game.countdown_clock().dump_state(out);

        // bind handler
        auto sender(std::bind(&json::write, std::ref(out), std::placeholders::_1));

        // send to clients
        sv.each_client(sender);
    }

    // bind handlers
    auto greeter(std::bind(&handle_new_client, std::placeholders::_1, std::ref(this->game)));
    auto handler(std::bind(&handle_client_input, std::placeholders::_1, std::ref(this->game)));

    // poll clients for commands, waiting at most 50ms
    return this->sv.poll(greeter, handler, 50000);
}