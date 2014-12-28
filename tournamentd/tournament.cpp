#include "tournament.hpp"
#include "stringcrc.hpp"
#include "datetime.hpp"
#include <cstddef>
#include <iostream>
#include <stdexcept>

// ----- read datetime from json (specialize here to not pollute json or datetime classes with each other)

template <>
bool json::get_value<datetime>(const char* name, datetime& value) const
{
    std::string str;
    if(this->get_value(name, str))
    {
        value = datetime::from_gm(str);
        return true;
    }

    return false;
}

// ----- command handlers

static void handle_cmd_authorize(std::unordered_set<int>& auths, json& out, const json& in)
{
    int code;
    if(in.get_value("authorize", code))
    {
        auths.insert(code);
        out.set_value("authorized", code);
    }
}

static void handle_cmd_version(const gameinfo& game, json& out)
{
    static const char* name = "tournamentd";
    static const char* version = "0.0.9";

    out.set_value("server_name", name);
    out.set_value("server_version", version);
}

static void handle_cmd_get_all_config(const gameinfo& game, json& out)
{
    game.dump_configuration(out);
}

static void handle_cmd_get_all_state(const gameinfo& game, json& out)
{
    game.dump_state(out);
}

static void handle_cmd_get_clock_state(const gameinfo& game, json& out)
{
    game.countdown_clock().dump_state(out);
}

static void handle_cmd_start_game(gameinfo& game, json& out, const json& in)
{
    datetime dt;
    if(in.get_value("start_at", dt))
    {
        game.countdown_clock().start(dt);
    }
    else
    {
        game.countdown_clock().start();
    }
}

void tournament::ensure_authorized(const json& in)
{
    int code;
    if(!in.get_value("authenticate", code) || auths.find(code) == this->auths.end())
    {
        throw std::runtime_error("unauthorized");
    }
}

// handler for new client
bool tournament::handle_new_client(std::ostream& client)
{
    // greet client
    json out;
    handle_cmd_version(this->game_info, out);
    handle_cmd_get_all_config(this->game_info, out);
    handle_cmd_get_all_state(this->game_info, out);
    client << out << std::endl;

    return false;
}

// handler for input from existing client
bool tournament::handle_client_input(std::iostream& client)
{
    // get a line of input
    std::string input;
    std::getline(client, input);

    // find start of command
    static const char* whitespace(" \t\r\n");
    auto cmd0(input.find_first_not_of(whitespace));
    if(cmd0 != std::string::npos)
    {
        // build up output
        json out;

        try
        {
            // parse command and argument
            std::string cmd;
            json in;

            // find end of command
            auto cmd1(input.find_first_of(whitespace, cmd0));
            if(cmd1 != std::string::npos)
            {
                auto arg0(input.find_first_not_of(whitespace, cmd1));
                if(arg0 != std::string::npos)
                {
                    auto arg = input.substr(arg0, std::string::npos);
                    in = json(arg);
                }
            }
            cmd = input.substr(cmd0, cmd1);

            // convert command to lower-case for hashing (use ::tolower, assuming ASCII-encoded input)
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

            // call command handler
            switch(crc32(cmd))
            {
                case crc32_("quit"):
                case crc32_("exit"):
                    return true;

                case crc32_("authorize"):
                    ensure_authorized(in);
                    handle_cmd_authorize(this->auths, out, in);
                    break;

                case crc32_("version"):
                    handle_cmd_version(this->game_info, out);
                    break;

                case crc32_("get_all_config"):
                    handle_cmd_get_all_config(this->game_info, out);
                    break;

                case crc32_("get_all_state"):
                    handle_cmd_get_all_state(this->game_info, out);
                    break;

                case crc32_("start_game"):
                    ensure_authorized(in);
                    handle_cmd_start_game(this->game_info, out, in);
                    break;

                default:
                    throw std::runtime_error("unknown command");
            }
        }
        catch(const std::exception& e)
        {
            out.set_value("error", e.what());
        }

        client << out << std::endl;
    }

    return false;
}

// handler for async game events
bool tournament::handle_game_event(std::ostream& client)
{
    json out;
    handle_cmd_get_clock_state(this->game_info, out);
    client << out << std::endl;
    return false;
}

bool tournament::run()
{
    // various handler callback function objects
    static const auto greeter(std::bind(&tournament::handle_new_client, this, std::placeholders::_1));
    static const auto handler(std::bind(&tournament::handle_client_input, this, std::placeholders::_1));
    static const auto sender(std::bind(&tournament::handle_game_event, this, std::placeholders::_1));

    // update the clock, and report to clients if anything changed
    if(this->game_info.countdown_clock().update_remaining())
    {
        // send to clients
        this->sv.each_client(sender);
    }

    // poll clients for commands, waiting at most 50ms
    return this->sv.poll(greeter, handler, 50000);
}