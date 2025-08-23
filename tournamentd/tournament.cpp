#include "tournament.hpp"
#include "gameinfo.hpp"
#include "logger.hpp"
#include "nlohmann/json.hpp"
#include "scope_timer.hpp"
#include "server.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

// poll clients for commands, waiting at most 50ms
#if !defined SERVER_POLL_TIMEOUT
#define SERVER_POLL_TIMEOUT 50000
#endif

// default listen port for tournamentd
#if !defined(DEFAULT_PORT)
#define DEFAULT_PORT 25600
#endif

struct tournament::impl
{
    // game object
    gameinfo game_info;

    // server to handle remote connections
    server game_server;

    // accepted authorization codes
    std::unordered_map<int,td::authorized_client> game_auths;

    // snapshot path
    std::string snapshot_path;

    // ----- auth check

    bool code_authorized(int code) const
    {
        return (this->game_auths.find(code) != this->game_auths.end());
    }

    void ensure_authorized(const nlohmann::json& in) const
    {
        if(!this->code_authorized(in.at("authenticate")))
        {
            throw td::protocol_error("unauthorized");
        }
    }

    std::vector<td::authorized_client> all_auths() const
    {
        std::vector<td::authorized_client> auths;
        auths.reserve(this->game_auths.size());
        for(auto kv : this->game_auths)
        {
            auths.push_back(kv.second);
        }
        return auths;
    }

    // ----- broadcast helpers

    void broadcast_state() const
    {
        nlohmann::json bcast;
        this->game_info.dump_state(bcast);
        this->game_info.dump_configuration_state(bcast);
        this->game_info.dump_derived_state(bcast);
        this->game_server.broadcast(bcast.dump());
    }

    // ----- command handlers available to anyone

    void handle_cmd_version(nlohmann::json& out) const
    {
        out["server_name"] = "tournamentd";
        out["server_version"] = "0.0.9";
    }

    void handle_cmd_get_config(nlohmann::json& out) const
    {
        // pass auth codes back into output
        out["authorized_clients"] = this->all_auths();
        this->game_info.dump_configuration(out);
    }

    void handle_cmd_get_state(nlohmann::json& out) const
    {
        this->game_info.dump_state(out);
        this->game_info.dump_configuration_state(out);
        this->game_info.dump_derived_state(out);
    }

    void handle_cmd_check_authorized(const nlohmann::json& in, nlohmann::json& out) const
    {
        auto code(in.at("authenticate"));
        auto authorized(this->code_authorized(code));
        logger(ll::debug) << "code " << code << " is " << (authorized ? "authorized\n" : "not authorized\n");
        out["authorized"] = authorized;
    }

    void handle_cmd_chips_for_buyin(const nlohmann::json& in, nlohmann::json& out) const
    {
        auto chips(this->game_info.chips_for_buyin(in.at("source_id"), in.at("max_expected_players")));
        out["chips_for_buyin"] = chips;
    }

    // ----- command handlers available to authorized clients

    void handle_cmd_configure(const nlohmann::json& in, nlohmann::json& out)
    {
        // handle auth codes. game_info doesn't handle these
        // read auth codes from input
        auto ac_it(in.find("authorized_clients"));
        if(ac_it != in.end())
        {
            std::vector<td::authorized_client> auths_vector(ac_it->begin(), ac_it->end());
            for(auto& auth : auths_vector)
            {
                logger(ll::debug) << "authorizing code " << auth.code << " named \"" << auth.name << "\"\n";
                this->game_auths.emplace(auth.code, auth);
            }
        }

        // pass auth codes back into output
        out["authorized_clients"] = this->all_auths();

        // configure
        this->game_info.configure(in);
        this->game_info.dump_configuration(out);
    }

    void handle_cmd_start_game(const nlohmann::json& in, nlohmann::json& /* out */)
    {
        auto start_at_it(in.find("start_at"));
        if(start_at_it != in.end())
        {
            this->game_info.start(*start_at_it);
        }
        else
        {
            this->game_info.start();
        }
    }

    void handle_cmd_stop_game(const nlohmann::json& /* in */, nlohmann::json& /* out */)
    {
        this->game_info.stop();
    }

    void handle_cmd_resume_game(const nlohmann::json& /* in */, nlohmann::json& /* out */)
    {
        this->game_info.resume();
    }

    void handle_cmd_pause_game(const nlohmann::json& /* in */, nlohmann::json& /* out */)
    {
        this->game_info.pause();
    }

    void handle_cmd_toggle_pause_game(const nlohmann::json& /* in */, nlohmann::json& /* out */)
    {
        this->game_info.toggle_pause_resume();
    }

    void handle_cmd_set_previous_level(const nlohmann::json& /* in */, nlohmann::json& out)
    {
        auto blind_level_changed(this->game_info.previous_blind_level());
        out["blind_level_changed"] = blind_level_changed;
    }

    void handle_cmd_set_next_level(const nlohmann::json& /* in */, nlohmann::json& out)
    {
        auto blind_level_changed(this->game_info.next_blind_level());
        out["blind_level_changed"] = blind_level_changed;
    }

    void handle_cmd_set_action_clock(const nlohmann::json& in, nlohmann::json& /* out */)
    {
        auto duration_it(in.find("duration"));
        if(duration_it != in.end())
        {
            this->game_info.set_action_clock(*duration_it);
        }
        else
        {
            this->game_info.reset_action_clock();
        }
    }

    void handle_cmd_gen_blind_levels(const nlohmann::json& in, nlohmann::json& out)
    {
        auto levels(this->game_info.gen_blind_levels(in.at("desired_duration"),
                                                     in.at("level_duration"),
                                                     in.value("expected_buyins", std::size_t{0}),
                                                     in.value("expected_rebuys", std::size_t{0}),
                                                     in.value("expected_addons", std::size_t{0}),
                                                     in.value("break_duration", 0L),
                                                     in.value("antes", td::ante_type_t::none),
                                                     in.value("ante_sb_ratio", 0.2)));
        out["blind_levels"] = levels;
    }

    void handle_cmd_reset_state(const nlohmann::json& /* in */, nlohmann::json& /* out */)
    {
        this->game_info.reset_state();
    }

    void handle_cmd_fund_player(const nlohmann::json& in, nlohmann::json& /* out */)
    {
        this->game_info.fund_player(in.at("player_id"), in.at("source_id"));
    }

    void handle_cmd_plan_seating(const nlohmann::json& in, nlohmann::json& out)
    {
        auto movements(this->game_info.plan_seating(in.at("max_expected_players")));
        out["players_moved"] = movements;
    }

    void handle_cmd_seat_player(const nlohmann::json& in, nlohmann::json& out)
    {
        auto seating(this->game_info.add_player(in.at("player_id")));
        out[seating.first] = seating.second;
    }

    void handle_cmd_unseat_player(const nlohmann::json& in, nlohmann::json& /* out */)
    {
        this->game_info.remove_player(in.at("player_id"));
    }

    void handle_cmd_bust_player(const nlohmann::json& in, nlohmann::json& out)
    {
        auto movements(this->game_info.bust_player(in.at("player_id")));
        out["players_moved"] = movements;
    }

    void handle_cmd_rebalance_seating(const nlohmann::json& /* in */, nlohmann::json& out)
    {
        auto movements(this->game_info.rebalance_seating());
        out["players_moved"] = movements;
    }

    void handle_cmd_quick_setup(const nlohmann::json& in, nlohmann::json& out)
    {
        std::vector<td::seated_player> seated_players;

        auto source_it(in.find("source_id"));
        if(source_it != in.end())
        {
            seated_players = this->game_info.quick_setup(*source_it);
        }
        else
        {
            seated_players = this->game_info.quick_setup();
        }
        out["seated_players"] = seated_players;
    }

    // handler for new client
    bool handle_new_client(std::ostream& /* client */) const
    {
        return false;
    }

    // handler for input from existing client
    bool handle_client_input(std::iostream& client)
    {
        std::string input;
        // get a line of input
        //
        // note: this is probably the most problematic line of the entire codebase.
        //
        // we want to consume all available lines of input, but we do not want to block if there is no input ready.
        // with just an if() statement, we only read the first line available and ignore the rest.
        // with just a while() statement, we keep trying the read even when there is no input ready, blocking!
        // third try: check input availability first with peek(), only read if something is ready.
        // fourth try: peek() also tries to fill the buffer and will block. implement a non-blocking peek
        // fifth try: back to a single if() and getline(). moved the loop outside of handle_client_input
        if(std::getline(client, input))
        {
            // find start of command
            static const char* whitespace(" \t\r\n");
            auto cmd0(input.find_first_not_of(whitespace));
            if(cmd0 != std::string::npos)
            {
                // build up output
                nlohmann::json out;

                try
                {
                    scope_timer timer;

                    // find end of command
                    auto cmd1(input.find_first_of(whitespace, cmd0));
                    if(cmd1 != std::string::npos)
                    {
                        nlohmann::json in;
                        auto cmd(input.substr(cmd0, cmd1));
                        auto arg0(input.find_first_not_of(whitespace, cmd1));
                        if(arg0 != std::string::npos)
                        {
                            auto arg(input.substr(arg0, std::string::npos));
                            in = nlohmann::json::parse(arg);
                        }

                        // convert command to lower-case for hashing (use ::tolower, assuming ASCII-encoded input)
                        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

                        // copy "echo" attribute to output, if sent. This will allow clients to correlate requests with responses
                        auto echo_it(in.find("echo"));
                        if(echo_it != in.end())
                        {
                            out["echo"] = *echo_it;
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
                            return true;
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
                             funding_sources (array): Each valid source of funding for this tournament
                             blind_levels (array): Discription of each blind level
                             available_chips (array): Discription of each chip color and denomination
                             available_tables (array): Discription of each named table
                             payout_policy (integer): Policy for paying out players (0 = automatic, 1 = forced, 2 = depends on turnout)
                             payout_currency (string): Currency used for payouts
                             automatic_payouts (object): Parameters for automatic payout structure generation:
                                percent_seats_paid (float): Proportion of players (buyins) paid out
                                round_payouts (bool): Round payoffs to integer values
                                payout_shape (float): How "flat" to make the payout structure. 0 = all places paid the same, 1 = winner takes all
                                pay_the_bubble (float): How much to pay the bubble (usually 0)
                                pay_knockouts (float): How much to set aside for each knockout
                             forced_payouts (array): Force this array of payouts, regardless of number of players
                             manual_payouts (array): Manual payout definitions: number of players and an array of payouts, if missing, automatic payouts are calculated
                             previous_blind_level_hold_duration (integer): How long after round starts should prev command go to the previous round (rather than restart)? (ms)
                             rebalance_policy (integer): Policy for rebalancing tables (0 = manual, 1 = when unbalanced, 2 = shootout)
                             background_color (string): Suggested clock user interface color
                             final_table_policy (integer): Policy for moving players to the final table (0 = fill in, 1 = randomize)
                             */
                            this->ensure_authorized(in);
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
                             players_finished (array): busted player ids in reverse bust out order, no duplicates
                             bust_history (array): Busted player ids in bust out order, can contain duplicates due to rebuys
                             empty_seats (array): Empty seat assignments
                             table_count (integer): Number of tables currently playing
                             buyins (array): Player ids who are both currently seated and bought in
                             unique_entries (array): Player ids who at one point have bought in
                             entries (array): Player ids for each buyin or rebuy
                             payouts (array): Payout amounts for each place
                             total_chips (integer): Count of all tournament chips in play
                             total_cost (array): Sum total of all buyins, rebuys and addons, for each currency
                             total_commission (array): Sum total of all entry fees, for each currency
                             total_equity (double): Sum total of all payouts, in configured payout_currency
                             running (bool): True if the tournament is unpaused
                             current_blind_level (integer): Current blind level. 0 = planning stage
                             current_time (integer): Current time since epoch (milliseconds)
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
                             funding_sources (optional, array): Each valid source of funding for this tournament
                             blind_levels (optional, array): Discription of each blind level
                             available_chips (optional, array): Discription of each chip color and denomination
                             available_tables (optional, array): Discription of each named table
                             automatic_payouts (object): Parameters for automatic payout structure generation:
                                 percent_seats_paid (float): Proportion of players (buyins) paid out
                                 round_payouts (bool): Round payoffs to integer values
                                 payout_shape (float): How "flat" to make the payout structure. 0 = all places paid the same, 1 = winner takes all
                                 pay_the_bubble (float): How much to pay the bubble (usually 0)
                                 pay_knockouts (float): How much to set aside for each knockout
                             forced_payouts (array): Force this array of payouts, regardless of number of players
                             manual_payouts (optional, array): Manual payout definitions: number of players and an array of payouts, if missing, automatic payouts are calculated
                             previous_blind_level_hold_duration (optional, integer): How long after round starts should prev command go to the previous round (rather than restart)? (ms)

                             output:
                             (none)
                             */
                            this->ensure_authorized(in);
                            this->handle_cmd_configure(in, out);
                            this->broadcast_state();
                        }
                        else if(cmd == "reset_state")
                        {
                            /*
                             command:
                             reset_state

                             purpose:
                             Resets all game state to no results, seating, funding, and stops the clock

                             input:
                             authenticate (integer): Valid authentication code for a tournament admin

                             output:
                             (none)
                             */
                            this->ensure_authorized(in);
                            this->handle_cmd_reset_state(in, out);
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
                             desired_duration (integer): Desired total tournament length (milliseconds)
                             level_duration (integer): Uniform duraiton for each level (milliseconds)
                             chips_in_play (integer): Estimated number of total chips in play including buyins, rebuys, and addons
                             break_duration (optional, integer): Length of break whenever we can chip up (defaults to no break)
                             antes (optional, enum): 0: No ante, 1: Traditional ante, 2: Big Blind Ante
                             ante_sb_ratio (optional, float): Approx. ratio between ante and small blind (defaults to 1:5)

                             output:
                             (none)
                             */
                            this->ensure_authorized(in);
                            this->handle_cmd_gen_blind_levels(in, out);
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
                             players_moved (array): Any player movements that have to happen
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
                             player_seated (object): Player, table, and seat
                             - or -
                             already_seated (object): Player, table, and seat
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
                        else if(cmd == "rebalance_seating")
                        {
                            /*
                             command:
                             rebalance_seating

                             purpose:
                             Manually try to break and rebalance tables

                             input:
                             authenticate (integer): Valid authentication code for a tournament admin

                             output:
                             players_moved (array): Any player movements that have to happen
                             */
                            this->ensure_authorized(in);
                            this->handle_cmd_rebalance_seating(in, out);
                            this->broadcast_state();
                        }
                        else if(cmd == "quick_setup")
                        {
                            /*
                             command:
                             quick_setup

                             purpose:
                             Quickly get a game going. Plan for all players in roster, seat all players
                             and buy them in with the first configured uyin

                             input:
                             authenticate (integer): Valid authentication code for a tournament admin
                             source_id (optional,funding source id): Funding source to use (default: first one)

                             output:
                             seated_players (array): List of all players and their seats
                             */
                            this->ensure_authorized(in);
                            this->handle_cmd_quick_setup(in, out);
                            this->broadcast_state();
                        }
                        else
                        {
                            throw td::protocol_error("unknown command");
                        }
                    }
                }
                catch(const td::protocol_error& e)
                {
                    out["error"] = e.what();
                    logger(ll::warning) << "caught protocol error while processing command: " << e.what() << '\n';
                }
                catch(const std::exception& e)
                {
                    out["exception"] = e.what();
                    logger(ll::warning) << "caught a non protocol error exception while processing command: " << e.what() << '\n';
                }

                client << out << std::endl;
            }
        }

        return false;
    }

    int authorize(int code)
    {
        logger(ll::info) << "client " << code << " pre-authorized to administer this tournament\n";
        this->game_auths.emplace(code, td::authorized_client(code, "Pre-authorized client"));
        return code;
    }

    static const std::string get_snapshot_path()
    {
        // look in environment for better temp dir
        auto tmpdir(std::getenv("TMPDIR"));
        if(tmpdir != nullptr)
        {
            return std::string(tmpdir) + "/tournamentd.snapshot.json";
        }
        else
        {
            return "/tmp/tournamentd.snapshot.json";
        }

    }

    void load_snapshot()
    {
        try
        {
            // try loading existing snapshot (to recover after accidental exits, crashes, etc.
            this->load_configuration(this->snapshot_path);
            logger(ll::info) << "loaded snapshot from " << this->snapshot_path << '\n';
        }
        catch(const std::exception& e)
        {
            logger(ll::debug) << "did not load snapshot from " << this->snapshot_path << ": " << e.what() << '\n';
        }
    }

    void remove_snapshot()
    {
        // remove any snapshot
        std::remove(this->snapshot_path.c_str());
        logger(ll::info) << "removed snapshot at " << this->snapshot_path << " because we are cleanly shutting down";
    }

public:
    impl() : snapshot_path(get_snapshot_path())
    {
        this->load_snapshot();
    }

    ~impl()
    {
        this->remove_snapshot();
    }

    // listen on both unix socket and inet
    std::pair<std::string, int> listen(const char* unix_socket_directory, int inet_socket_port)
    {
        // build unique unix socket name using service name
        std::ostringstream inet_service;
        inet_service << inet_socket_port;

        if(unix_socket_directory != nullptr)
        {
            try
            {
                // build unique unix socket name using service name
                std::ostringstream local_server;
                local_server << unix_socket_directory << "/tournamentd." << inet_socket_port << ".sock";

                // try to listen to this service
                this->game_server.listen(local_server.str().c_str(), inet_service.str().c_str());
                return std::make_pair(local_server.str(), inet_socket_port);
            }
            catch(const std::system_error& e)
            {
                // EPERM: failed to bind due to permission issue. retry without unix socket
                if(e.code().value() != EPERM)
                {
                    throw;
                }

                logger(ll::warning) << "unix socket creation resulted in: " << e.what() << ". retrying without unix socket\n";
            }
        }

        // try to listen to this service, without a unix socket
        this->game_server.listen(nullptr, inet_service.str().c_str());
        return std::make_pair(std::string(), inet_socket_port);
    }

    // load configuration from file
    void load_configuration(const std::string& filename)
    {
        // TODO: Consider whether this should throw an exception for non-existent files
        // Currently it silently ignores missing files, which may not be the desired behavior
        std::ifstream config_stream(filename);
        if(config_stream.good())
        {
            nlohmann::json config;
            config_stream >> config;
            this->game_info.configure(config);
        }
    }

    // update/poll loop
    bool update_and_poll()
    {
        // update state
        this->game_info.update();

        // report to clients if running
        if(this->game_info.is_started())
        {
            scope_timer timer;
            timer.set_message("broadcast_state: ");

            // send to clients
            this->broadcast_state();
        }

        // poll clients for commands
        auto greeter(std::bind(&tournament::impl::handle_new_client, this, std::placeholders::_1));
        auto handler(std::bind(&tournament::impl::handle_client_input, this, std::placeholders::_1));
        auto quit(this->game_server.poll(greeter, handler, SERVER_POLL_TIMEOUT));

        // snapshot if state is dirty
        if(this->game_info.state_is_dirty())
        {
            // try opening the file
            std::ofstream snapshot_stream(this->snapshot_path);
            if(snapshot_stream.good())
            {
                snapshot_stream << std::setw(4);

                // get the snapshot (config + state)
                nlohmann::json snapshot;
                this->game_info.dump_configuration(snapshot);
                this->game_info.dump_state(snapshot);
                snapshot_stream << snapshot;

                logger(ll::info) << "saved snapshot to " << this->snapshot_path << '\n';
            }
        }

        // return whether or not the server should quit
        return quit;
    }
};

tournament::tournament() : pimpl(new impl)
{
}

tournament::~tournament() = default;

int tournament::authorize(int code)
{
    return this->pimpl->authorize(code);
}

// listen for clients on any available service, returning the unix socket path and port
std::pair<std::string, int> tournament::listen(const char* unix_socket_directory)
{
    // start at default port, and increment until we find one that binds
    for(int port(DEFAULT_PORT); port < DEFAULT_PORT+100; port++)
    {
        try
        {
            return this->pimpl->listen(unix_socket_directory, port);
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
    this->pimpl->load_configuration(filename);
}

bool tournament::run()
{
    try
    {
        return this->pimpl->update_and_poll();
    }
    catch(const std::system_error& e)
    {
        // EINTR: select() was interrupted. Just retry
        if(e.code().value() != EINTR)
        {
            logger(ll::error) << "caught system_error: " << e.what() << '\n';
            throw;
        }
    }
    catch(const std::exception& e)
    {
        logger(ll::error) << "unhandled exception: " << e.what() << '\n';
        throw;
    }

    return false;
}
