#include "program.hpp"
#include "bonjour.hpp"
#include "logger.hpp"
#include "nlohmann/json.hpp"
#include "socket.hpp"
#include "socketstream.hpp"
#include <iostream>
#include <random>

static socketstream make_stream(const std::string& server, const std::string port, const std::string& unix_path=std::string())
{
    if(unix_path.empty())
    {
        logger(ll::debug) << "connecting to " << server << ':' << port << '\n';
        return socketstream(inet4_socket(server.c_str(), port.c_str()));
    }
    else
    {
        logger(ll::debug) << "connecting to " << unix_path << '\n';
        return socketstream(unix_socket(unix_path.c_str(), true));
    }
}

static void send_command(std::iostream& stream, const std::string& cmd, const std::string& auth=std::string(), nlohmann::json arg=nlohmann::json())
{
    arg["echo"] = 31337;

    // add auth if exists
    if(!auth.empty())
    {
        arg["authenticate"] = std::stol(auth);
    }

    // send command with optional json argument
    if(arg.empty())
    {
        stream << cmd << '\r' << std::endl;
    }
    else
    {
        stream << cmd << ' ' << arg << '\r' << std::endl;
    }

    // read until response
    nlohmann::json res;
    while(true)
    {
        stream >> res;
        auto echo_it(res.find("echo"));
        if(echo_it != res.end() && *echo_it == 31337)
        {
            break;
        }
    }

    // Print all fields except "echo"
    for(auto it = res.begin(); it != res.end(); ++it) {
        if(it.key() != "echo") {
            std::cout << it.key() << ": " << it.value() << '\n';
        }
    }
}

struct program::impl
{
public:
    explicit impl(const std::vector<std::string>& cmdline)
    {
        std::string server("localhost");
        std::string port("25600");
        std::string unix_path;
        std::string auth;

        static const char* usage =
            "Usage: tournamentctl [options] command [arguments]\n"
            " -s, --server HOSTNAME\tConnect to sever (default: localhost)\n"
            " -p, --port PORT\tConnect to server port (default: 25600)\n"
            " -u, --unix PATH\tConnect to servier listening on unix socket\n"
            " -a, --auth CODE\tConnect using authorization code\n"
            "\n"
            " Commands:\n"
            "\n"
            " Basic Info:\n"
            "\tversion: Get tournamentd version\n"
            "\tcheck_authorized: Check if auth code is valid\n"
            "\tget_config: Get tournament configuration\n"
            "\tget_state: Get tournament state\n"
            "\n"
            " Tournament Setup and Planning:\n"
            "\tconfigure [config_json]: Configure tournament\n"
            "\tplan_seating <max_players>: Plan seating arrangement\n"
            "\tquick_setup [max_players]: Quick tournament setup\n"
            "\tgen_blind_levels <chips> <target_min> <level_min> <structure>: Generate blind structure\n"
            "\tfund_player <player_id> <source_id>: Fund player buy-in/rebuy/addon\n"
            "\n"
            " Tournament Control:\n"
            "\treset_state: Reset tournament to initial state\n"
            "\tstart_game: Start the tournament\n"
            "\tstop_game: Stop the tournament\n"
            "\tpause_game: Pause the tournament\n"
            "\tresume_game: Resume the tournament\n"
            "\ttoggle_pause_game: Toggle pause state\n"
            "\tset_previous_level: Move to previous blind level\n"
            "\tset_next_level: Move to next blind level\n"
            "\n"
            " Seating Control:\n"
            "\tseat_player <player_id>: Seat a player\n"
            "\tunseat_player <player_id>: Unseat a player\n"
            "\tbust_player <player_id>: Bust player from tournament\n"
            "\trebalance_seating: Rebalance table seating\n"
            "\n"
            " Action Clock Control:\n"
            "\tset_action_clock <duration_ms>: Set action clock\n"
            "\n"
            " Utilities:\n"
            "\tchips_for_buyin <source_id> <max_players>: Calculate chip distribution\n";

#if !defined(DEBUG)
        // disable ll::debug
        logger_enable(ll::info, ll::warning, ll::error);
#endif

        // parse command-line
        for(auto it(cmdline.begin()+1); it != cmdline.end();)
        {
            auto opt(*it++);

            if(opt == "-s" || opt == "--server")
            {
                if(it != cmdline.end())
                {
                    server = *it++;
                }
                else
                {
                    std::cerr << "No parameter for " << opt << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(opt == "-p" || opt == "--port")
            {
                if(it != cmdline.end())
                {
                    port = *it++;
                }
                else
                {
                    std::cerr << "No parameter for " << opt << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(opt == "-u" || opt == "--unix")
            {
                if(it != cmdline.end())
                {
                    unix_path = *it++;
                }
                else
                {
                    std::cerr << "No parameter for " << opt << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(opt == "-a" || opt == "--auth")
            {
                if(it != cmdline.end())
                {
                    auth = *it++;
                }
                else
                {
                    std::cerr << "No parameter for " << opt << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(opt == "-h" || opt == "--help")
            {
                std::cerr << usage;
                std::exit(EXIT_SUCCESS);
            }
            else
            {
                // make a stream, given server and port, or unix_path
                auto stream(make_stream(server, port, unix_path));

                // handle command
                if(opt == "bust_player" || opt == "seat_player" || opt == "unseat_player")
                {
                    // commands that need player_id
                    nlohmann::json arg;
                    arg["player_id"] = *it++;
                    send_command(stream, opt, auth, arg);
                }
                else if(opt == "fund_player")
                {
                    // needs player_id and source_id
                    nlohmann::json arg;
                    arg["player_id"] = *it++;
                    arg["source_id"] = std::stol(*it++);
                    send_command(stream, opt, auth, arg);
                }
                else if(opt == "plan_seating")
                {
                    // needs max_expected_players
                    nlohmann::json arg;
                    arg["max_expected_players"] = std::stol(*it++);
                    send_command(stream, opt, auth, arg);
                }
                else if(opt == "chips_for_buyin")
                {
                    // needs source_id and max_expected_players
                    nlohmann::json arg;
                    arg["source_id"] = std::stol(*it++);
                    arg["max_expected_players"] = std::stol(*it++);
                    send_command(stream, opt, auth, arg);
                }
                else if(opt == "set_action_clock")
                {
                    // needs clock duration in milliseconds
                    nlohmann::json arg;
                    arg["duration"] = std::stol(*it++);
                    send_command(stream, opt, auth, arg);
                }
                else if(opt == "gen_blind_levels")
                {
                    // needs starting_chips, target_duration, level_duration, structure
                    nlohmann::json arg;
                    arg["starting_chips"] = std::stol(*it++);
                    arg["target_duration"] = std::stol(*it++);
                    arg["level_duration"] = std::stol(*it++);
                    arg["structure"] = *it++;
                    send_command(stream, opt, auth, arg);
                }
                else if(opt == "quick_setup")
                {
                    // optional max_expected_players
                    nlohmann::json arg;
                    if(it != cmdline.end()) {
                        arg["max_expected_players"] = std::stol(*it++);
                    }
                    send_command(stream, opt, auth, arg);
                }
                else
                {
                    // all other commands with optional arg
                    nlohmann::json arg = (it != cmdline.end()) ? nlohmann::json(*it++) : nlohmann::json();
                    send_command(stream, opt, auth, arg);
                }
            }
        }
    }
};

program::program(const std::vector<std::string>& cmdline) : pimpl(new impl(cmdline))
{
}

program::~program() = default;

bool program::run()
{
    return true;
}

bool program::sigusr2()
{
    return false;
}
