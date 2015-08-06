#include "program.hpp"
#include "stringcrc.hpp"
#include <sstream>

program::program(const std::vector<std::string>& cmdline)
{
    std::string name("tournamentd");

#if defined(DEBUG)
    // debug only: always accept this code
    this->tourney.authorize(31337);

    // debug only: load a default configuration
    this->tourney.load_configuration("defaults.json");
#endif

    // parse command-line
    for(auto& it(cmdline.begin()+1); it != cmdline.end();)
    {
        switch(crc32(*it++))
        {
            case crc32_("-c"):
            case crc32_("--conf"):
                if(it != cmdline.end())
                {
                    // load supplied config
                    this->tourney.load_configuration(*it++);
                }
                break;

            case crc32_("-a"):
            case crc32_("--auth"):
                if(it != cmdline.end())
                {
                    // parse client code
                    this->tourney.authorize(std::stoi(*it++));
                }
                break;

            case crc32_("-n"):
            case crc32_("--name"):
                if(it != cmdline.end())
                {
                    name = *it++;
                }
                break;

            case crc32_("-h"):
            case crc32_("--help"):
            default:
                std::cerr << "Unknown option: " << *it << "\n"
                             "Usage: tournamentd [options]\n"
                             " -c, --conf FILE\tInitialize configuration from file\n"
                             " -a, --auth LIST\tPre-authorize client authentication code\n"
                             " -n, --name NAME\tPublish Bonjour service with given name (default: tournamentd)\n";
                std::exit(EXIT_FAILURE);
                break;
        }
    }

    // listen and publish
    this->publisher.publish(name, this->tourney.listen(P_tmpdir).second);
}

bool program::run()
{
    return this->tourney.run();
}

bool program::sigusr2()
{
    return false;
}
