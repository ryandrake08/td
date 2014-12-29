#include "tournament.hpp"
#include "stringcrc.hpp"
#include "datetime.hpp"
#include "json.hpp"
#include <cstddef>
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

// ----- auth check

void tournament::ensure_authorized(const json& in) const
{
    int code;
    if(!in.get_value("authenticate", code) || this->game_auths.find(code) == this->game_auths.end())
    {
        throw std::runtime_error("unauthorized");
    }
}

// ----- command handlers available to anyone

void tournament::handle_cmd_version(json& out) const
{
    static const char* name = "tournamentd";
    static const char* version = "0.0.9";

    out.set_value("server_name", name);
    out.set_value("server_version", version);
}

void tournament::handle_cmd_get_config(json& out) const
{
    this->game_info.dump_configuration(out);
    this->clock.dump_configuration(out);
    this->funding.dump_configuration(out);
    this->seating.dump_configuration(out);
}

void tournament::handle_cmd_get_state(json& out) const
{
    this->clock.dump_state(out);
    this->funding.dump_state(out);
    this->seating.dump_state(out);
}

// ----- command handlers available to authorized clients

void tournament::handle_cmd_authorize(json& out, const json& in)
{
    int code;
    if(in.get_value("authorize", code))
    {
        this->game_auths.insert(code);
        out.set_value("authorized", code);
    }
}

void tournament::handle_cmd_start_game(const json& in)
{
    datetime dt;
    if(in.get_value("start_at", dt))
    {
        this->clock.start(dt);
    }
    else
    {
        this->clock.start();
    }

    json out;
    this->clock.dump_state(out);
    this->game_server.broadcast(out.string());
}

void tournament::handle_cmd_stop_game(const json& in)
{
    this->clock.stop();

    json out;
    this->clock.dump_state(out);
    this->game_server.broadcast(out.string());
}

void tournament::handle_cmd_resume_game(const json& in)
{
    this->clock.resume();

    json out;
    this->clock.dump_state(out);
    this->game_server.broadcast(out.string());
}

void tournament::handle_cmd_pause_game(const json& in)
{
    this->clock.pause();

    json out;
    this->clock.dump_state(out);
    this->game_server.broadcast(out.string());
}

void tournament::handle_cmd_previous_level(const json& in)
{
    if(this->clock.previous_blind_level())
    {
        json out;
        this->clock.dump_state(out);
        this->game_server.broadcast(out.string());
    }
}

void tournament::handle_cmd_next_level(const json& in)
{
    if(this->clock.next_blind_level())
    {
        json out;
        this->clock.dump_state(out);
        this->game_server.broadcast(out.string());
    }
}

void tournament::handle_cmd_set_action_clock(const json& in)
{
    long duration;
    if(in.get_value("duration", duration))
    {
        this->clock.set_action_clock(duration);
    }
    else
    {
        this->clock.reset_action_clock();
    }

    json out;
    this->clock.dump_state(out);
    this->game_server.broadcast(out.string());
}

void tournament::handle_cmd_gen_blind_levels(const json& in)
{
    std::size_t count(30); // default to 30 blind levels
    long duration(3600000); // default to 1 hour levels

    in.get_value("duration", duration);
    in.get_value("count", count);

    this->clock.gen_blind_levels(count, duration);

    json out;
    this->clock.dump_configuration(out);
    this->game_server.broadcast(out.string());
}

// handler for new client
bool tournament::handle_new_client(std::ostream& client) const
{
    // greet client
    json out;
    this->handle_cmd_version(out);
    this->handle_cmd_get_config(out);
    this->handle_cmd_get_state(out);
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
                    this->ensure_authorized(in);
                    this->handle_cmd_authorize(out, in);
                    break;

                case crc32_("version"):
                    this->handle_cmd_version(out);
                    break;

                case crc32_("get_config"):
                    this->handle_cmd_get_config(out);
                    break;

                case crc32_("get_state"):
                    this->handle_cmd_get_state(out);
                    break;

                case crc32_("start_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_start_game(in);
                    break;

                case crc32_("stop_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_stop_game(in);
                    break;

                case crc32_("resume_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_resume_game(in);
                    break;

                case crc32_("pause_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_pause_game(in);
                    break;

                case crc32_("previous_level"):
                    this->ensure_authorized(in);
                    this->handle_cmd_previous_level(in);
                    break;

                case crc32_("next_level"):
                    this->ensure_authorized(in);
                    this->handle_cmd_next_level(in);
                    break;

                case crc32_("set_action_clock"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_action_clock(in);
                    break;

                case crc32_("gen_blind_levels"):
                    this->ensure_authorized(in);
                    this->handle_cmd_gen_blind_levels(in);
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

// load configuration from file
void tournament::load_configuration(const std::string& filename)
{
    auto config(json::load(filename));
    this->game_info.configure(config);
    this->clock.configure(config);
    this->funding.configure(config);
    this->seating.configure(config);
}

bool tournament::run()
{
    // various handler callback function objects
    static const auto greeter(std::bind(&tournament::handle_new_client, this, std::placeholders::_1));
    static const auto handler(std::bind(&tournament::handle_client_input, this, std::placeholders::_1));

    // update the clock, and report to clients if anything changed
    if(this->clock.update_remaining())
    {
        // send to clients
        json out;
        this->clock.dump_state(out);
        this->game_server.broadcast(out.string());
    }

    // poll clients for commands, waiting at most 50ms
    return this->game_server.poll(greeter, handler, 50000);
}