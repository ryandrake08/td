#include "program.hpp"
#include "bonjour.hpp"
#include "logger.hpp"
#include "nlohmann/json.hpp"
#include "socket.hpp"
#include "socketstream.hpp"
#include <iostream>

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
            res.erase("echo");
            break;
        }
    }

    // Print all fields except "echo"
    for(auto& [key, value] : res.items()) {
        std::cout << key << ": " << value << '\n';
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
            "\tversion: Get tournamentd version\n";

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
                if(opt == "bust_player")
                {
                    // prepare argument
                    nlohmann::json arg;
                    arg["player_id"] = *it++;
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
