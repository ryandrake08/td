#include "program.hpp"
#include "stringcrc.hpp"

program::program(const std::vector<std::string>& cmdline)
{
    std::string service("25600");

#if defined(DEBUG)
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
                             " -p, --port NUMBER\tListen on given port (default: 23000)\n"
                             " -a, --auth LIST\tPre-authorize client authentication code\n";
                std::exit(EXIT_FAILURE);
                break;
        }
    }

    // listen on appropriate port
    this->tourney.listen(service.c_str());
}

bool program::run()
{
    return this->tourney.run();
}