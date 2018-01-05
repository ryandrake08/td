#include "program.hpp"
#include "bonjour.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "socket.hpp"
#include "socketstream.hpp"
#include <iostream>

static socketstream make_stream(const std::string& server, const std::string port, const std::string& unix_path=std::string())
{
    if(unix_path.empty())
    {
        logger(LOG_DEBUG) << "connecting to " << server << ':' << port << '\n';
        return socketstream(inet4_socket(server.c_str(), port.c_str()));
    }
    else
    {
        logger(LOG_DEBUG) << "connecting to " << unix_path << '\n';
        return socketstream(unix_socket(unix_path.c_str(), true));
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
        // disable LOG_DEBUG
        logger_enable(LOG_INFO, LOG_WARNING, LOG_ERROR);
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
                // treat the rest as the command and arguments

                // make a stream, given server and port, or unix_path
                auto stream(make_stream(server, port, unix_path));

                // build initial command argument
                json arg;
                if(!auth.empty())
                {
                    arg.set_value("authenticate", auth);
                }

                // handle command
                if(opt == "version")
                {
                    // send command
                    stream << "version " << arg << '\r' << std::endl;

                    // read response
                    std::string input;
                    if(std::getline(stream, input))
                    {
                        // parse the line from server
                        auto object(json::eval(input));

                        // parse line
                        std::string name, version;
                        if(object.get_value("server_name", name))
                        {
                            std::cout << "name: " << name << '\n';
                        }
                        if(object.get_value("server_version", version))
                        {
                            std::cout << "version: " << version << '\n';
                        }
                    }
                }
                else
                {
                    std::cerr << "Unknown command: " << opt << '\n';
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
