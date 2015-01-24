#include "program.hpp"
#include "stringcrc.hpp"
#include <sstream>

program::program(const std::vector<std::string>& cmdline)
{
    std::string service;

#if defined(DEBUG)
    // debug only: default to listening on inet service
    service = "25600";
    
    // debug only: always accept this code
    this->tourney.authorize(31337);

    // debug only: load a default configuration
    this->tourney.load_configuration("defaults.json");
#endif

    // parse command-line
    for(auto it(cmdline.begin()+1); it != cmdline.end(); it++)
    {
        switch(crc32(*it))
        {
            case crc32_("-c"):
            case crc32_("--conf"):
                if(++it != cmdline.end())
                {
                    // load supplied config
                    this->tourney.load_configuration(*it);
                }
                break;

            case crc32_("-p"):
            case crc32_("--port"):
                if(++it != cmdline.end())
                {
                    // parse port number
                    service = *it;
                }
                else
                {
                    service = "25600";
                }
                break;

            case crc32_("-a"):
            case crc32_("--auth"):
                if(++it != cmdline.end())
                {
                    // parse client code
                    this->tourney.authorize(std::stoi(*it));
                }
                break;

            case crc32_("-h"):
            case crc32_("--help"):
            default:
                std::cerr << "Unknown option: " << *it << "\n"
                             "Usage: tournamentd [options]\n"
                             " -c, --conf FILE\tInitialize configuration from file\n"
                             " -p, --port [NUMBER]\tListen on given port (default: 25600)\n"
                             " -a, --auth LIST\tPre-authorize client authentication code\n";
                std::exit(EXIT_FAILURE);
                break;
        }
    }

    if(service.empty())
    {
        // listen only on a default unix socket
        this->tourney.listen("/tmp/tournamentd.sock");
    }
    else
    {
        // build unique unix socket name using service name
        std::ostringstream ss;
        ss << "/tmp/tournamentd." << service << ".sock";

        // listen on both unix socket and service
        this->tourney.listen(ss.str().c_str(), service.c_str());
    }
}

bool program::run()
{
    return this->tourney.run();
}

bool program::sigusr2()
{
    return false;
}
