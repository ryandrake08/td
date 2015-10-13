#include "tournament.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "scope_timer.hpp"
#include "socketstream.hpp"
#include <algorithm>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <system_error>

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

    out.set_value("chips_for_buyin", json(chips.begin(), chips.end()));
}

// ----- command handlers available to authorized clients

void tournament::handle_cmd_configure(const json& in, json& out)
{
    // read auth codes from input
    int mycode;
    std::vector<td::authorized_client> auths_vector;
    if(in.get_values("authorized_clients", auths_vector) && in.get_value("authenticate", mycode))
    {
        this->game_auths.clear();
        for(auto& auth : auths_vector)
        {
            logger(LOG_DEBUG) << "Authorizing code " << auth.code << " named \"" << auth.name << "\"\n";
            this->game_auths.emplace(auth.code, auth);
        }

        // always make sure caller is in the authorization list
        logger(LOG_DEBUG) << "Authorizing myself (code " << mycode << ")\n";
        this->game_auths.emplace(mycode, mycode);
    }

    // configure
    this->game_info.configure(in);
    this->game_info.dump_configuration(out);

    // inject auth codes back into output
    out.set_value("authorized_clients", json(this->game_auths.begin(), this->game_auths.end()));
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

void tournament::handle_cmd_reset_funding(const json &in, json &out)
{
    this->game_info.reset_funding();
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

    out.set_value("players_moved", json(movements.begin(), movements.end()));
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

    std::string input;
    // get lines of input
    //
    // note: this is probably the most problematic line of the entire codebase.
    //
    // we want to consume all available lines of input, but we do not want to block if there is no input ready.
    // with just an if() statement, we only read the first line available and ignore the rest.
    // with just a while() statement, we keep trying the read even when there is no input ready, blocking!
    // third try: check input availability first with peek(), only read if something is ready.
    // fourth try: peek() also tries to fill the buffer and will block. implement a non-blocking peek
    const socketstream& socket_client(dynamic_cast<const socketstream&>(client));
    while((ret == false) && (socket_client.nonblocking_peek() != EOF) && std::getline(client, input))
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
                scope_timer timer;

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

                // set message for timer
                timer.set_message("command " + cmd + " handled in: ");

                // call command handler
                if(cmd == "quit" || cmd == "exit")
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
                    ret = true;
                }
                else if(cmd == "check_authorized")
                {
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
                    this->handle_cmd_check_authorized(in, out);
                }
                else if(cmd == "version")
                {
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
                    this->handle_cmd_version(out);
                }
                else if(cmd == "get_config")
                {
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
                        previous_blind_level_hold_duration (integer): How long after round starts should prev command go to the previous round (rather than restart)? (ms)
                     */
                    this->handle_cmd_get_config(out);
                }
                else if(cmd == "get_state")
                {
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
                        action_clock_time_remaining (integer): Time remaining on action clock (milliseconds)
                        elapsed (integer): Tournament time elapsed (milliseconds)
                     */
                    this->handle_cmd_get_state(out);
                }
                else if(cmd == "chips_for_buyin")
                {
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
                    this->handle_cmd_chips_for_buyin(in, out);
                }
                else if(cmd == "configure")
                {
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
                        previous_blind_level_hold_duration (optional, integer): How long after round starts should prev command go to the previous round (rather than restart)? (ms)

                     output:
                        (none)
                     */
                    this->ensure_authorized(in);
                    this->handle_cmd_configure(in, out);
                    this->broadcast_configuration();
                    this->broadcast_state();
                }
                else if(cmd == "start_game")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_start_game(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "stop_game")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_stop_game(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "resume_game")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_resume_game(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "pause_game")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_pause_game(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "toggle_pause_game")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_toggle_pause_game(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "set_previous_level")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_set_previous_level(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "set_next_level")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_set_next_level(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "set_action_clock")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_set_action_clock(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "gen_blind_levels")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_gen_blind_levels(in, out);
                    this->broadcast_configuration();
                }
                else if(cmd == "reset_funding")
                {
                    /*
                     command:
                        reset_funding

                     purpose:
                        Zero out all buyins, entries, cash and chips

                     input:
                        authenticate (integer): Valid authentication code for a tournament admin

                     output:
                        (none)
                     */
                    this->ensure_authorized(in);
                    this->handle_cmd_reset_funding(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "fund_player")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_fund_player(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "plan_seating")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_plan_seating(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "seat_player")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_seat_player(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "unseat_player")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_unseat_player(in, out);
                    this->broadcast_state();
                }
                else if(cmd == "bust_player")
                {
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
                    this->ensure_authorized(in);
                    this->handle_cmd_bust_player(in, out);
                    this->broadcast_state();
                }
                else
                {
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
    this->game_auths.emplace(code, code);
    return code;
}

#if !defined(DEFAULT_PORT)
#define DEFAULT_PORT 25600
#endif

// listen for clients on any available service, returning the unix socket path and port
std::pair<std::string, int> tournament::listen(const char* unix_socket_directory)
{
    // start at default port, and increment until we find one that binds
    for(int port(DEFAULT_PORT); port < DEFAULT_PORT+100; port++)
    {
        // build unique unix socket name using service name
        std::ostringstream inet_service;
        inet_service << port;

        try
        {
			if (unix_socket_directory == nullptr)
			{
				// try to listen to this service
				this->game_server.listen(nullptr, inet_service.str().c_str());
				return std::make_pair("", port);
			}
			else
			{
				// build unique unix socket name using service name
				std::ostringstream local_server;
				local_server << unix_socket_directory << "/tournamentd." << port << ".sock";

				// try to listen to this service
				this->game_server.listen(local_server.str().c_str(), inet_service.str().c_str());
				return std::make_pair(local_server.str(), port);
			}
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
            scope_timer("broadcast_state: ");

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
