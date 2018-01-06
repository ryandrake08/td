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

static json send_command(std::iostream& stream, const std::string& cmd, const std::string& auth=std::string(), json arg=json())
{
    // add auth if exists
    if(!auth.empty())
    {
        arg.set_value("authenticate", std::stol(auth));
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

    // read response
    std::string response;
    if(std::getline(stream, response))
    {
        // convert line from server to json and return
        return json::eval(response);
    }
    else
    {
        return json();
    }
}

static void print_value_if_exists(const json& object, const std::string& name)
{
    json value;
    if(object.get_value(name.c_str(), value))
    {
        std::cout << name << ": " << value << '\n';
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
                // make a stream, given server and port, or unix_path
                auto stream(make_stream(server, port, unix_path));

                // handle command
                if(opt == "version")
                {
                    // send command
                    auto object(send_command(stream, opt));

                    // print output object
                    print_value_if_exists(object, "server_name");
                    print_value_if_exists(object, "server_version");
                }
                else if(opt == "check_authorized")
                {
                    // send command
                    auto object(send_command(stream, opt, auth));

                    // print output object
                    print_value_if_exists(object, "authorized");
                }
                else if(opt == "get_config")
                {
                    // send command
                    auto object(send_command(stream, opt));

                    // print output raw json
                    std::cout << object << '\n';
                }
                else if(opt == "get_state")
                {
                    // send command
                    auto object(send_command(stream, opt));

                    // print output raw json
                    std::cout << object << '\n';
                }
                else
                {
                    // arbitrary command with optional arg
                    if(it == cmdline.end())
                    {
                        auto object(send_command(stream, opt, auth));

                        // print output raw json
                        std::cout << object << '\n';
                    }
                    else
                    {
                        auto object(send_command(stream, opt, auth, json::eval(*it++)));

                        // print output raw json
                        std::cout << object << '\n';
                    }
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
