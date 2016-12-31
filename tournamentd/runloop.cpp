
#include "runloop.hpp"
#include "program.hpp"
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <vector>

volatile static sig_atomic_t signal_caught = 0;

void runloop::signal_handler(int signum)
{
    (void) signal(signum, signal_handler);
    signal_caught = signum;
    std::cerr << "runloop: caught signal " << signum << std::endl;
}

runloop::runloop()
{
    // Initialize signal handlers
    (void) signal(SIGINT, signal_handler);
    (void) signal(SIGTERM, signal_handler);
#if defined(SIGUSR1)
    (void) signal(SIGUSR1, signal_handler);
#endif
#if defined(SIGUSR2)
    (void) signal(SIGUSR2, signal_handler);
#endif
#if defined(SIGPIPE)
    (void) signal(SIGPIPE, SIG_IGN);
#endif
}

runloop::~runloop()
{
    // Shutdown signal handlers
    (void) signal(SIGINT, SIG_DFL);
    (void) signal(SIGTERM, SIG_DFL);
#if defined(SIGUSR1)
    (void) signal(SIGUSR1, SIG_DFL);
#endif
#if defined(SIGUSR2)
    (void) signal(SIGUSR2, SIG_DFL);
#endif
#if defined(SIGPIPE)
    (void) signal(SIGPIPE, SIG_DFL);
#endif
}

int runloop::run(int argc, const char* const argv[])
{
    // Gather command line
    std::vector<std::string> cmdline;
    std::copy(&argv[0], &argv[argc], std::back_inserter(cmdline));

    bool restarting(false);
    do
    {
        restarting = false;

        program c(cmdline);

        while(signal_caught == 0)
        {
            if(c.run())
            {
                return EXIT_SUCCESS;
            }

#if defined(SIGUSR2)
            // Notify program of SIGUSR2 so it can take some custom action (on systems that support SIGUSR2)
            if(signal_caught == SIGUSR2)
            {
                if(c.sigusr2())
                {
                    return EXIT_SUCCESS;
                }

                // Clear signal_caught to continue calling run loop
                signal_caught = 0;
            }
#endif
        }

#if defined(SIGUSR1)
        // Restart gracefully on SIGUSR1 (on systems that support SIGUSR1)
        if(signal_caught == SIGUSR1)
        {
            restarting = true;

            // Clear signal_caught for next time
            signal_caught = 0;
        }
#endif
    }
    while(restarting);
    return EXIT_SUCCESS;
}
