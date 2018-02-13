#include "program.hpp"
#include "bonjour.hpp"
#include "logger.hpp"
#include "tournament.hpp"

#if !defined(P_tmpdir)
#define    P_tmpdir "/tmp/"
#endif

struct program::impl
{
    bonjour_publisher publisher;
    tournament tourney;

public:
    explicit impl(const std::vector<std::string>& cmdline)
    {
        std::string name("tournamentd");

        static const char* usage =
            "Usage: tournamentd [options]\n"
            " -c, --conf FILE\tInitialize configuration from file\n"
            " -a, --auth CODE\tPre-authorize client authentication code\n"
            " -n, --name NAME\tPublish Bonjour service with given name (default: tournamentd)\n";

#if defined(DEBUG)
        // debug only: always accept this code
        this->tourney.authorize(31337);

        // debug only: load a default configuration
        this->tourney.load_configuration("defaults.json");
#else
        // disable ll::debug
        logger_enable(ll::info, ll::warning, ll::error);
#endif

        // parse command-line
        for(auto it(cmdline.begin()+1); it != cmdline.end();)
        {
            auto cmd(*it++);

            if(cmd == "-c" || cmd == "--conf")
            {
                if(it != cmdline.end())
                {
                    // load supplied config
                    this->tourney.load_configuration(*it++);
                }
                else
                {
                    std::cerr << "No parameter for " << cmd << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(cmd == "-a" || cmd == "--auth")
            {
                if(it != cmdline.end())
                {
                    // parse client code
                    this->tourney.authorize(std::stoi(*it++));
                }
                else
                {
                    std::cerr << "No parameter for " << cmd << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(cmd == "-n" || cmd == "--name")
            {
                if(it != cmdline.end())
                {
                    name = *it++;
                }
                else
                {
                    std::cerr << "No parameter for " << cmd << "\n" << usage;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(cmd == "-h" || cmd == "--help")
            {
                std::cerr << usage;
                std::exit(EXIT_SUCCESS);
            }
            else
            {
                std::cerr << "Unknown option: " << *it << "\n" << usage;
                std::exit(EXIT_FAILURE);
            }
        }

        // listen and publish
        this->publisher.publish(name.c_str(), this->tourney.listen(P_tmpdir).second);
    }

    bool run()
    {
        return this->tourney.run();
    }

    bool sigusr2()
    {
        logger(ll::info) << "user signal\n";
        return false;
    }
};

program::program(const std::vector<std::string>& cmdline) : pimpl(new impl(cmdline))
{
}

program::~program() = default;

bool program::run()
{
    return this->pimpl->run();
}

bool program::sigusr2()
{
    return this->pimpl->sigusr2();
}
