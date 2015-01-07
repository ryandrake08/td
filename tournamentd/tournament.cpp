#include "tournament.hpp"
#include "stringcrc.hpp"
#include "json.hpp"
#include <algorithm>
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

// ----- broadcast helpers

template <typename T>
void tournament::broadcast_state(const T& object) const
{
    json bcast;
    object.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

template <typename T>
void tournament::broadcast_configuration(const T& object) const
{
    json bcast;
    object.dump_configuration(bcast);
    this->game_server.broadcast(bcast.string());
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

void tournament::handle_cmd_check_authorized(const json& in, json& out)
{
    int code;
    if(!in.get_value("authenticate", code))
    {
        throw std::invalid_argument("must specify authentication code");
    }

    if(this->game_auths.find(code) == this->game_auths.end())
    {
        out.set_value("authorized", false);
    }
    else
    {
        out.set_value("authorized", true);
    }
}

// ----- command handlers available to authorized clients

void tournament::handle_cmd_authorize(const json& in, json& out)
{
    int code;
    if(!in.get_value("authorize", code))
    {
        throw std::invalid_argument("must specify a code to authorize");
    }

    code = this->authorize(code);
    
    out.set_value("authorized_client", code);
}

void tournament::handle_cmd_start_game(const json& in, json& out)
{
    datetime start_at;
    if(in.get_value("start_at", start_at))
    {
        this->clock.start(start_at);
    }
    else
    {
        this->clock.start();
    }
}

void tournament::handle_cmd_stop_game(const json& in, json& out)
{
    this->clock.stop();
}

void tournament::handle_cmd_resume_game(const json& in, json& out)
{
    this->clock.resume();
}

void tournament::handle_cmd_pause_game(const json& in, json& out)
{
    this->clock.pause();
}

void tournament::handle_cmd_set_previous_level(const json& in, json& out)
{
    auto blind_level_changed(this->clock.previous_blind_level());

    out.set_value("blind_level_changed", blind_level_changed);
}

void tournament::handle_cmd_set_next_level(const json& in, json& out)
{
    auto blind_level_changed(this->clock.next_blind_level());

    out.set_value("blind_level_changed", blind_level_changed);
}

void tournament::handle_cmd_set_action_clock(const json& in, json& out)
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
}

void tournament::handle_cmd_gen_blind_levels(const json& in, json& out)
{
    std::size_t count;
    long duration;

    if(!in.get_value("duration", duration) || !in.get_value("count", count))
    {
        throw std::invalid_argument("must specify count and duration");
    }

    this->clock.gen_blind_levels(count, duration);
}

void tournament::handle_cmd_reset_funding(const json& in, json& out)
{
    this->funding.reset();
}

void tournament::handle_cmd_fund_player(const json& in, json& out)
{
    td::player_id player;
    td::funding_source_id source;

    if(!in.get_value("player", player) || !in.get_value("source_id", source))
    {
        throw std::invalid_argument("must specify player and source");
    }

    this->funding.fund_player(player, source, this->clock.get_current_blind_level());
}

void tournament::handle_cmd_plan_seating(const json& in, json& out)
{
    std::size_t max_expected_players;

    if(!in.get_value("max_expected_players", max_expected_players))
    {
        throw std::invalid_argument("must specify max_expected_players");
    }

    (void) this->seating.plan_seating(max_expected_players);
}

void tournament::handle_cmd_seat_player(const json& in, json& out)
{
    td::player_id player;

    if(!in.get_value("player", player))
    {
        throw std::invalid_argument("must specify player");
    }

    auto seating(this->seating.add_player(player));

    json player_seated;
    player_seated.set_value("player_id", player);
    player_seated.set_value("table_number", seating.table_number);
    player_seated.set_value("seat_number", seating.seat_number);
    out.set_value("player_seated", player_seated);
}

void tournament::handle_cmd_bust_player(const json& in, json& out)
{
    td::player_id player;

    if(!in.get_value("player", player))
    {
        throw std::invalid_argument("must specify player");
    }

    auto movements(this->seating.remove_player(player));

    out.set_values("players_moved", movements);
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
                cmd = input.substr(cmd0, cmd1);
                auto arg0(input.find_first_not_of(whitespace, cmd1));
                if(arg0 != std::string::npos)
                {
                    auto arg = input.substr(arg0, std::string::npos);
                    in = json::eval(arg);
                }
            }

            // convert command to lower-case for hashing (use ::tolower, assuming ASCII-encoded input)
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

            // copy "echo" attribute to output, if sent. This will allow clients to correlate requests with responses
            json echo;
            if(in.get_value("echo", echo))
            {
                out.set_value("echo", echo);
            }

            // call command handler
            switch(crc32(cmd))
            {
                case crc32_("quit"):
                case crc32_("exit"):
                    return true;

                case crc32_("authorize"):
                    this->ensure_authorized(in);
                    this->handle_cmd_authorize(in, out);
                    break;

                case crc32_("check_authorized"):
                    this->handle_cmd_check_authorized(in, out);
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
                    this->handle_cmd_start_game(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("stop_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_stop_game(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("resume_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_resume_game(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("pause_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_pause_game(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("set_previous_level"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_previous_level(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("set_next_level"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_next_level(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("set_action_clock"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_action_clock(in, out);
                    this->broadcast_state(this->clock);
                    break;

                case crc32_("gen_blind_levels"):
                    this->ensure_authorized(in);
                    this->handle_cmd_gen_blind_levels(in, out);
                    this->broadcast_configuration(this->clock);
                    break;

                case crc32_("reset_funding"):
                    this->ensure_authorized(in);
                    this->handle_cmd_reset_funding(in, out);
                    this->broadcast_state(this->funding);
                    break;

                case crc32_("fund_player"):
                    this->ensure_authorized(in);
                    this->handle_cmd_fund_player(in, out);
                    this->broadcast_state(this->funding);
                    break;

                case crc32_("plan_seating"):
                    this->ensure_authorized(in);
                    this->handle_cmd_plan_seating(in, out);
                    this->broadcast_state(this->seating);
                    break;

                case crc32_("seat_player"):
                    this->ensure_authorized(in);
                    this->handle_cmd_seat_player(in, out);
                    this->broadcast_state(this->seating);
                    break;

                case crc32_("bust_player"):
                    this->ensure_authorized(in);
                    this->handle_cmd_bust_player(in, out);
                    this->broadcast_state(this->seating);
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

int tournament::authorize(int code)
{
    this->game_auths.insert(code);
    return code;
}

// listen for clients on given port
void tournament::listen(const char* unix_socket_path, const char* service)
{
    this->game_server.listen(unix_socket_path, service);
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