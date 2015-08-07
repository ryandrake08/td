#include "tournament.hpp"
#include "stringcrc.hpp"
#include "json.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cstddef>
#include <sstream>
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

template <>
json& json::set_value(const char* name, const datetime& value)
{
    this->set_value(name, value.gmtime());
    return *this;
}

// ----- auth check

void tournament::ensure_authorized(const json& in) const
{
    int code;
    if(!in.get_value("authenticate", code) || this->game_auths.find(code) == this->game_auths.end())
    {
        throw td::protocol_error("unauthorized");
    }
}

// ----- broadcast helpers

void tournament::broadcast_state() const
{
    json bcast;
    game_info.dump_state(bcast);
    this->game_server.broadcast(bcast.string());
}

void tournament::broadcast_configuration() const
{
    json bcast;
    game_info.dump_configuration(bcast);
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
}

void tournament::handle_cmd_get_state(json& out) const
{
    this->game_info.dump_state(out);
}

void tournament::handle_cmd_check_authorized(const json& in, json& out) const
{
    int code;
    if(!in.get_value("authenticate", code))
    {
        throw td::protocol_error("must specify authentication code");
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

void tournament::handle_cmd_chips_for_buyin(const json& in, json& out) const
{
    td::funding_source_id_t source;
    if(!in.get_value("source_id", source))
    {
        throw td::protocol_error("must specify source");
    }

    std::size_t max_expected_players;
    if(!in.get_value("max_expected_players", max_expected_players))
    {
        throw td::protocol_error("must specify max_expected_players");
    }

    auto chips(this->game_info.chips_for_buyin(source, max_expected_players));

    out.set_values("chips_for_buyin", chips);
}

// ----- command handlers available to authorized clients

void tournament::handle_cmd_authorize(const json& in, json& out)
{
    // read auth codes
    int mycode;
    std::vector<int> auths_vector;
    if(in.get_value("authorize", auths_vector) && in.get_value("authenticate", mycode))
    {
        this->game_auths = std::unordered_set<int>(auths_vector.begin(), auths_vector.end());

        // always make sure caller is in the authorization list
        this->game_auths.insert(mycode);
    }

    out.set_value("authorized_clients", json::json(this->game_auths));
}

void tournament::handle_cmd_configure(const json& in, json& out)
{
    this->game_info.configure(in);
    this->game_info.dump_configuration(out);
}

void tournament::handle_cmd_start_game(const json& in, json& out)
{
    datetime start_at;
    if(in.get_value("start_at", start_at))
    {
        this->game_info.start(start_at);
    }
    else
    {
        this->game_info.start();
    }
}

void tournament::handle_cmd_stop_game(const json& in, json& out)
{
    this->game_info.stop();
}

void tournament::handle_cmd_resume_game(const json& in, json& out)
{
    this->game_info.resume();
}

void tournament::handle_cmd_pause_game(const json& in, json& out)
{
    this->game_info.pause();
}

void tournament::handle_cmd_toggle_pause_game(const json& in, json& out)
{
    this->game_info.toggle_pause_resume();
}

void tournament::handle_cmd_set_previous_level(const json& in, json& out)
{
    auto blind_level_changed(this->game_info.previous_blind_level());

    out.set_value("blind_level_changed", blind_level_changed);
}

void tournament::handle_cmd_set_next_level(const json& in, json& out)
{
    auto blind_level_changed(this->game_info.next_blind_level());

    out.set_value("blind_level_changed", blind_level_changed);
}

void tournament::handle_cmd_set_action_clock(const json& in, json& out)
{
    long duration;
    if(in.get_value("duration", duration))
    {
        this->game_info.set_action_clock(duration);
    }
    else
    {
        this->game_info.reset_action_clock();
    }
}

void tournament::handle_cmd_gen_blind_levels(const json& in, json& out)
{
    std::size_t count;
    long duration;
    long break_duration;
    double blind_increase_factor;

    if(!in.get_value("count", count) ||
       !in.get_value("duration", duration) ||
       !in.get_value("break_duration", break_duration) ||
       !in.get_value("blind_increase_factor", blind_increase_factor))
    {
        throw td::protocol_error("must specify count and duration");
    }

    this->game_info.gen_blind_levels(count, duration, break_duration, blind_increase_factor);
}

void tournament::handle_cmd_fund_player(const json& in, json& out)
{
    td::player_id_t player_id;
    td::funding_source_id_t source_id;

    if(!in.get_value("player_id", player_id) || !in.get_value("source_id", source_id))
    {
        throw td::protocol_error("must specify player and source");
    }

    this->game_info.fund_player(player_id, source_id, this->game_info.get_current_blind_level());
}

void tournament::handle_cmd_plan_seating(const json& in, json& out)
{
    std::size_t max_expected_players;

    if(!in.get_value("max_expected_players", max_expected_players))
    {
        throw td::protocol_error("must specify max_expected_players");
    }

    (void) this->game_info.plan_seating(max_expected_players);
}

void tournament::handle_cmd_seat_player(const json& in, json& out)
{
    td::player_id_t player_id;

    if(!in.get_value("player_id", player_id))
    {
        throw td::protocol_error("must specify player");
    }

    auto seating(this->game_info.add_player(player_id));

    json player_seated;
    player_seated.set_value("player_id", player_id);
    player_seated.set_value("table_number", seating.table_number);
    player_seated.set_value("seat_number", seating.seat_number);
    out.set_value("player_seated", player_seated);
}

void tournament::handle_cmd_unseat_player(const json& in, json& out)
{
    td::player_id_t player_id;

    if(!in.get_value("player_id", player_id))
    {
        throw td::protocol_error("must specify player");
    }

    auto seating(this->game_info.remove_player(player_id));

    json player_unseated;
    player_unseated.set_value("player_id", player_id);
    player_unseated.set_value("table_number", seating.table_number);
    player_unseated.set_value("seat_number", seating.seat_number);
    out.set_value("player_unseated", player_unseated);
}

void tournament::handle_cmd_bust_player(const json& in, json& out)
{
    td::player_id_t player_id;

    if(!in.get_value("player_id", player_id))
    {
        throw td::protocol_error("must specify player");
    }

    auto movements(this->game_info.bust_player(player_id));

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
    bool ret(false);

    // get a line of input
    std::string input;
    if(std::getline(client, input))
    {
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
                        /*
                         command:
                            quit (or exit)

                         purpose:
                            Disconnect client cleanly
                         
                         input:
                            (none)
                         
                         output:
                            (none)
                        */
                    case crc32_("quit"):
                    case crc32_("exit"):
                        ret = true;
                        break;

                        /*
                         command:
                            authorize

                         purpose:
                            Authorize codes to administer the tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            authorize (array): Authentication codes to give admin powers to

                         output:
                            authorized_clients (array): Authentication codes that are valid for administration
                         */
                    case crc32_("authorize"):
                        this->ensure_authorized(in);
                        this->handle_cmd_authorize(in, out);
                        break;

                        /*
                         command:
                            check_authorized

                         purpose:
                            Check whether a code is authorized to administer the tournament

                         input:
                            authenticate (integer): Authentication code to check

                         output:
                            authorized (bool): True if code is valid for administration (warning this can be exploited, should turn this off)
                         */
                    case crc32_("check_authorized"):
                        this->handle_cmd_check_authorized(in, out);
                        break;

                        /*
                         command:
                            version
                         
                         purpose:
                            Dump the server's version info

                         input:
                            (none)

                         output:
                            server_name (string): "tournamentd"
                            server_version (string): Description of server's API version
                        */
                    case crc32_("version"):
                        this->handle_cmd_version(out);
                        break;

                        /*
                         command:
                            get_config

                         purpose:
                            Dump the server's current configuration

                         input:
                            (none)

                         output:
                            name (string): Human-readable name of this tournament
                            players (array): Each player eligible for this tournament
                            table_capacity (integer): Number of seats per table
                            cost_currency (string): Currency name for buyins, re-buys, etc. ISO 4217: USD, EUR, XXX for points
                            equity_currency (string): Currency name for equity/payouts. ISO 4217: USD, EUR, XXX for points
                            percent_seats_paid (float): Proportion of players (buyins) paid out
                            round_payouts (bool): Round payoffs to integer values
                            payout_flatness (float): How "flat" to make the payout structure
                            funding_sources (array): Each valid source of funding for this tournament
                            blind_levels (array): Discription of each blind level
                            available_chips (array): Discription of each chip color and denomination
                            manual_payouts (array): Manual payout definitions: number of players and an array of payouts
                         */
                    case crc32_("get_config"):
                        this->handle_cmd_get_config(out);
                        break;

                        /*
                         command:
                            get_state

                         purpose:
                            Dump the server's current game state

                         input:
                            (none)

                         output:
                            seats (array): Seat assignment for each player id
                            players_finished (array): Player ids without seats (busted)
                            empty_seats (array): Empty seat assignments
                            tables (integer): Number of tables currently playing
                            buyins (array): Player ids who have bought in at least once
                            entries (array): Player ids for each buyin or rebuy
                            payouts (array): Payout amounts for each place
                            total_chips (integer): Count of all tournament chips in play
                            total_cost (float): Sum total of all buyins, rebuys and addons
                            total_commission (float): Sum total of all entry fees
                            total_equity (float): Sum total of all payouts
                            running (bool): True if the tournament is unpaused
                            current_blind_level (integer): Current blind level. 0 = planning stage
                            time_remaining (integer): Time remaining in current level (milliseconds)
                            break_time_remaining (integer): Time remaining in current break (milliseconds)
                            action_clock_remaining (integer): Time remaining on action clock (milliseconds)
                            elapsed (integer): Tournament time elapsed (milliseconds)
                         */
                    case crc32_("get_state"):
                        this->handle_cmd_get_state(out);
                        break;

                        /*
                         command:
                            chips_for_buyin

                         purpose:
                            Given configured chip set and expected number of players, calculate the quantity of each chip needed for starting stack

                         input:
                            source_id (funding source id): Funding source to calculate for
                            max_expected_players (integer): Number of players expected in the tournament

                         output:
                            chips_for_buyin (array): Quantities for each chip denomination
                         */
                    case crc32_("chips_for_buyin"):
                        this->handle_cmd_chips_for_buyin(in, out);
                        break;

                        /*
                         command:
                            configure

                         purpose:
                            Load a configuration into the tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            name (optional, string): Human-readable name for this tournament
                            players (optional, array): Each player eligible for this tournament
                            table_capacity (optional, integer): Number of seats per table
                            cost_currency (optional, string): Currency name for buyins, re-buys, etc. ISO 4217: USD, EUR, XXX for points
                            equity_currency (optional, string): Currency name for equity/payouts. ISO 4217: USD, EUR, XXX for points
                            percent_seats_paid (optional, float): Proportion of players (buyins) paid out
                            round_payouts (optional, bool): Round payoffs to integer values
                            payout_flatness (optinoal, float): How "flat" to make the payout structure
                            funding_sources (optional, array): Each valid source of funding for this tournament
                            blind_levels (optional, array): Discription of each blind level
                            available_chips (optional, array): Discription of each chip color and denomination
                            manual_payouts (optional, array): Manual payout definitions: number of players and an array of payouts

                         output:
                            (none)
                         */
                    case crc32_("configure"):
                        this->ensure_authorized(in);
                        this->handle_cmd_configure(in, out);
                        this->broadcast_configuration();
                        break;

                        /*
                         command:
                            start_game

                         purpose:
                            Start the tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            start_at (optional, date): Time to start the tournament

                         output:
                            (none)
                         */
                    case crc32_("start_game"):
                        this->ensure_authorized(in);
                        this->handle_cmd_start_game(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            stop_game

                         purpose:
                            Stop the tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin

                         output:
                            (none)
                         */
                    case crc32_("stop_game"):
                        this->ensure_authorized(in);
                        this->handle_cmd_stop_game(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            resume_game

                         purpose:
                            Resume a paused tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin

                         output:
                            (none)
                         */
                    case crc32_("resume_game"):
                        this->ensure_authorized(in);
                        this->handle_cmd_resume_game(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            pause_game

                         purpose:
                            Pause the tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin

                         output:
                            (none)
                         */
                    case crc32_("pause_game"):
                        this->ensure_authorized(in);
                        this->handle_cmd_pause_game(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            toggle_pause_game

                         purpose:
                            Pause the tournament if running, unpause if not

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin

                         output:
                            (none)
                         */
                    case crc32_("toggle_pause_game"):
                        this->ensure_authorized(in);
                        this->handle_cmd_toggle_pause_game(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            set_previous_level

                         purpose:
                            Set the tournament back one level (unless tournament is in the first round, or it's been <2 seconds since set back)

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin

                         output:
                            blind_level_changed (bool): True if the blind level actually changed
                         */
                    case crc32_("set_previous_level"):
                        this->ensure_authorized(in);
                        this->handle_cmd_set_previous_level(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            set_next_level

                         purpose:
                            Set the tournament forward one level (unless tournament is in the last round)

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin

                         output:
                            blind_level_changed (bool): True if the blind level actually changed
                         */
                    case crc32_("set_next_level"):
                        this->ensure_authorized(in);
                        this->handle_cmd_set_next_level(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            set_action_clock

                         purpose:
                            Call the clock on a player, starting a countdown timer

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            duration (optional, integer): Duration of countdown (milliseconds). If not set, clears the countdown

                         output:
                            (none)
                         */
                    case crc32_("set_action_clock"):
                        this->ensure_authorized(in);
                        this->handle_cmd_set_action_clock(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            gen_blind_levels

                         purpose:
                            Generate progressive blind levels, given available chip denominations

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            count (integer): Number of blind levels to generate
                            duration (integer): Uniform duraiton for each level (milliseconds)
                            break_duration (integer): If not zero, add a break whenever we can chip up
                            blind_increase_factor (float): Approx. amount to multiply to increase blinds each round (1.5 is usually good here)

                         output:
                            (none)
                         */
                    case crc32_("gen_blind_levels"):
                        this->ensure_authorized(in);
                        this->handle_cmd_gen_blind_levels(in, out);
                        this->broadcast_configuration();
                        break;

                        /*
                         command:
                            fund_player

                         purpose:
                            Accept a buyin, rebuy, or addon for a player

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            player_id (player id): Player to fund
                            source_id (funding source id): Chosen funding source

                         output:
                            (none)
                         */
                    case crc32_("fund_player"):
                        this->ensure_authorized(in);
                        this->handle_cmd_fund_player(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            plan_seating

                         purpose:
                            Generate an empty, random seating plan, given number of players

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            max_expected_players (integer): Maximum number of players expected

                         output:
                            (none)
                         */
                    case crc32_("plan_seating"):
                        this->ensure_authorized(in);
                        this->handle_cmd_plan_seating(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            seat_player

                         purpose:
                            Seat a player in the next available seat

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            player_id (player id): Player to seat

                         output:
                            player_seated (object): Player, seat, and table numbers
                         */
                    case crc32_("seat_player"):
                        this->ensure_authorized(in);
                        this->handle_cmd_seat_player(in, out);
                        this->broadcast_state();
                        break;

                        /*
                         command:
                            unseat_player

                         purpose:
                            Unseat a player without busting him (as if player was never in)

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            player_id (player id): Player to unseat

                         output:
                            (none)
                         */
                    case crc32_("unseat_player"):
                        this->ensure_authorized(in);
                        this->handle_cmd_unseat_player(in, out);
                        this->broadcast_state();
                        break;
                        
                        /*
                         command:
                            bust_player

                         purpose:
                            Bust a player out of the tournament

                         input:
                            authenticate (integer): Valid authentication code for a tournament admin
                            player_id (player id): Player to bust

                         output:
                            players_moved (array): Any player movements that have to happen (rebalancing)
                         */
                    case crc32_("bust_player"):
                        this->ensure_authorized(in);
                        this->handle_cmd_bust_player(in, out);
                        this->broadcast_state();
                        break;

                    default:
                        throw td::protocol_error("unknown command");
                }
            }
            catch(const td::protocol_error& e)
            {
                out.set_value("error", e.what());
                logger(LOG_WARNING) << "caught protocol error while processing command: " << e.what() << '\n';
            }

            client << out << std::endl;
        }
    }

    return ret;
}

int tournament::authorize(int code)
{
    this->game_auths.insert(code);
    return code;
}

#if !defined(DEFAULT_PORT)
#define DEFAULT_PORT 25600
#endif

// listen for clients on any available service, returning the unix socket path and port
std::pair<std::string, int> tournament::listen(const char*  unix_socket_directory)
{
    // start at default port, and increment until we find one that binds
    for(int port(DEFAULT_PORT); port < DEFAULT_PORT+100; port++)
    {
        // build unique unix socket name using service name
        std::ostringstream local_server, inet_service;
        local_server << unix_socket_directory << "tournamentd." << port << ".sock";
        inet_service << port;

        try
        {
            // try to listen to this service
            this->game_server.listen(local_server.str().c_str(), inet_service.str().c_str());
            return std::make_pair(local_server.str(), port);
        }
        catch(const std::system_error& e)
        {
            // EADDRINUSE: failed to bind, probably another server on this port
            if(e.code().value() == EADDRINUSE)
            {
                continue;
            }

            // re-throw anything not
            throw;
        }
    }

    // fail
    return std::pair<std::string,int>();
}

// load configuration from file
void tournament::load_configuration(const std::string& filename)
{
    auto config(json::load(filename));
    this->game_info.configure(config);
}

bool tournament::run()
{
    try
    {
        // various handler callback function objects
        auto greeter(std::bind(&tournament::handle_new_client, this, std::placeholders::_1));
        auto handler(std::bind(&tournament::handle_client_input, this, std::placeholders::_1));

        // update the clock, and report to clients if anything changed
        if(this->game_info.update_remaining())
        {
            // send to clients
            this->broadcast_state();
        }

        // poll clients for commands, waiting at most 50ms
        return this->game_server.poll(greeter, handler, 50000);
    }
    catch(const std::system_error& e)
    {
        // EINTR: select() was interrupted. Just retry
        if(e.code().value() != EINTR)
        {
            logger(LOG_ERROR) << "caught system_error: " << e.what() << '\n';
            throw;
        }
    }
    catch(const std::exception& e)
    {
        logger(LOG_ERROR) << "unhandled exception: " << e.what() << '\n';
        throw;
    }

    return false;
}