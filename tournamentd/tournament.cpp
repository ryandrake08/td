#include "tournament.hpp"
#include "stringcrc.hpp"
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

// ----- game structure speciailization

template <>
json& json::set_value(const char* name, const std::vector<gameseating::player_movement>& values)
{
    std::vector<json> array;
    for(auto value : values)
    {
        json obj;
        obj.set_value("player_id", value.player);
        obj.set_value("from_table_number", value.from_seat.table_number);
        obj.set_value("from_seat_number", value.from_seat.seat_number);
        obj.set_value("to_table_number", value.to_seat.table_number);
        obj.set_value("to_seat_number", value.to_seat.seat_number);
        array.push_back(obj);
    }
    return this->set_value(name, array);
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

void tournament::handle_cmd_authorize(const json& in, json& out)
{
    int code;
    if(!in.get_value("authorize", code))
    {
        throw std::invalid_argument("must specify a code to authorize");
    }

    code = this->authorize(code);

    out.set_value("authorized", code);
}

void tournament::handle_cmd_start_game(const json& in, json& out)
{
    datetime start_at;
    if(in.get_value("start_at", start_at))
    {
        this->start_game(start_at);
    }
    else
    {
        this->start_game();
    }
}

void tournament::handle_cmd_stop_game(const json& in, json& out)
{
    this->stop_game();
}

void tournament::handle_cmd_resume_game(const json& in, json& out)
{
    this->resume_game();
}

void tournament::handle_cmd_pause_game(const json& in, json& out)
{
    this->pause_game();
}

void tournament::handle_cmd_set_previous_level(const json& in, json& out)
{
    auto current_blind_level(this->set_previous_level());

    out.set_value("current_blind_level", current_blind_level);
}

void tournament::handle_cmd_set_next_level(const json& in, json& out)
{
    auto current_blind_level(this->set_next_level());

    out.set_value("current_blind_level", current_blind_level);
}

void tournament::handle_cmd_set_action_clock(const json& in, json& out)
{
    long duration;
    if(!in.get_value("duration", duration))
    {
        throw std::invalid_argument("must specify duration");
    }

    this->set_action_clock(duration);
}

void tournament::handle_cmd_reset_action_clock(const json& in, json& out)
{
    this->reset_action_clock();
}

void tournament::handle_cmd_gen_blind_levels(const json& in, json& out)
{
    std::size_t count;
    long duration;

    if(!in.get_value("duration", duration) || !in.get_value("count", count))
    {
        throw std::invalid_argument("must specify count and duration");
    }

    this->gen_blind_levels(count, duration);
}

void tournament::handle_cmd_reset_funding(const json& in, json& out)
{
    this->reset_funding();
}

void tournament::handle_cmd_fund_player(const json& in, json& out)
{
    player_id player;
    funding_source_id source;

    if(!in.get_value("player", player) || !in.get_value("source_id", source))
    {
        throw std::invalid_argument("must specify player and source");
    }

    this->fund_player(player, source);
}

void tournament::handle_cmd_plan_seating(const json& in, json& out)
{
    std::size_t max_expected_players;

    if(!in.get_value("max_expected_players", max_expected_players))
    {
        throw std::invalid_argument("must specify max_expected_players");
    }

    auto tables(this->plan_seating(max_expected_players));

    out.set_value("tables", tables);
}

void tournament::handle_cmd_seat_player(const json& in, json& out)
{
    player_id player;

    if(!in.get_value("player", player))
    {
        throw std::invalid_argument("must specify player");
    }

    auto seating(this->seat_player(player));

    out.set_value("table_number", seating.table_number);
    out.set_value("seat_number", seating.seat_number);
}

void tournament::handle_cmd_bust_player(const json& in, json& out)
{
    player_id player;

    if(!in.get_value("player", player))
    {
        throw std::invalid_argument("must specify player");
    }

    auto movements(this->bust_player(player));

    out.set_value("movements", movements);
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
                    this->handle_cmd_authorize(in, out);
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
                    break;

                case crc32_("stop_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_stop_game(in, out);
                    break;

                case crc32_("resume_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_resume_game(in, out);
                    break;

                case crc32_("pause_game"):
                    this->ensure_authorized(in);
                    this->handle_cmd_pause_game(in, out);
                    break;

                case crc32_("set_previous_level"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_previous_level(in, out);
                    break;

                case crc32_("set_next_level"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_next_level(in, out);
                    break;

                case crc32_("set_action_clock"):
                    this->ensure_authorized(in);
                    this->handle_cmd_set_action_clock(in, out);
                    break;

                case crc32_("gen_blind_levels"):
                    this->ensure_authorized(in);
                    this->handle_cmd_gen_blind_levels(in, out);
                    break;

                case crc32_("fund_player"):
                    this->ensure_authorized(in);
                    this->handle_cmd_fund_player(in, out);
                    break;

                case crc32_("plan_seating"):
                    this->ensure_authorized(in);
                    this->handle_cmd_plan_seating(in, out);
                    break;

                case crc32_("seat_player"):
                    this->ensure_authorized(in);
                    this->handle_cmd_seat_player(in, out);
                    break;

                case crc32_("bust_player"):
                    this->ensure_authorized(in);
                    this->handle_cmd_bust_player(in, out);
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

void tournament::start_game(const datetime& start_at)
{
    this->clock.start(start_at);

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::start_game()
{
    this->clock.start();

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::stop_game()
{
    this->clock.stop();

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::resume_game()
{
    this->clock.resume();

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::pause_game()
{
    this->clock.pause();

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

std::size_t tournament::set_previous_level()
{
    auto ret(this->clock.previous_blind_level());

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());

    return ret;
}

std::size_t tournament::set_next_level()
{
    auto ret(this->clock.next_blind_level());

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());

    return ret;
}

void tournament::set_action_clock(long duration_ms)
{
    this->clock.set_action_clock(duration_ms);

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::reset_action_clock()
{
    this->clock.reset_action_clock();

    json bcast;
    this->clock.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::gen_blind_levels(std::size_t count, long level_duration_ms)
{
    this->clock.gen_blind_levels(count, level_duration_ms);

    json bcast;
    this->clock.dump_configuration(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::reset_funding()
{
    this->funding.reset();

    json bcast;
    this->funding.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::fund_player(player_id player, funding_source_id source)
{
    this->funding.fund_player(player, source, this->clock.get_current_blind_level());

    json bcast;
    this->funding.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

std::size_t tournament::plan_seating(std::size_t max_expected_players)
{
    auto ret(this->seating.plan_seating(max_expected_players));

    json bcast;
    this->seating.dump_state(bcast);
    this->game_server.broadcast(bcast.string());

    return ret;
}

gameseating::seat tournament::seat_player(player_id player)
{
    auto ret(this->seating.add_player(player));

    json bcast;
    this->seating.dump_state(bcast);
    this->game_server.broadcast(bcast.string());

    return ret;
}

std::vector<gameseating::player_movement> tournament::bust_player(player_id player)
{
    auto ret(this->seating.remove_player(player));

    json bcast;
    this->seating.dump_state(bcast);
    this->game_server.broadcast(bcast.string());

    return ret;
}

// listen for clients on given port
void tournament::listen(std::uint16_t port)
{
    this->game_server.listen(port);
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