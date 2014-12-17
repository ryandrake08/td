#include "program.hpp"
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <cerrno> // For EINTR
#include <exception>
#include <iostream>
#include <vector>

#if !defined(SIGUSR1) && defined(SIGBREAK)
#define SIGUSR1 SIGBREAK
#endif

class runloop
{
    // Signal handling
    volatile static sig_atomic_t signal_caught;
    static void signal_handler(int signum)
    {
        (void) signal(signum, signal_handler);
        signal_caught = signum;
        std::cerr << "runloop: caught signal " << signum << std::endl;
    }

public:
    runloop()
    {
        // Initialize signal handlers
        (void) signal(SIGINT, signal_handler);
        (void) signal(SIGTERM, signal_handler);
        (void) signal(SIGUSR1, signal_handler);
    }

    ~runloop()
    {
        // Shutdown signal handlers
        (void) signal(SIGINT, SIG_DFL);
        (void) signal(SIGTERM, SIG_DFL);
        (void) signal(SIGUSR1, SIG_DFL);
    }

    int run(int argc, const char* const argv[])
    {
        // Gather command line
        std::vector<std::string> cmdline;
        std::copy(&argv[0], &argv[argc], std::back_inserter(cmdline));

        bool restarting;
        do
        {
            program c(cmdline);

            while(signal_caught == 0)
            {
                try
                {
                    auto ret(c.run());
                    if(ret)
                    {
                        return EXIT_FAILURE;
                    }
                }
                catch(const std::system_error& e)
                {
                    // EINTR: select() was interrupted. Just retry
                    if(e.code().value() != EINTR)
                    {
                        throw;
                    }
                }
            }

            // Restart gracefully on SIGUSR1
            restarting = signal_caught == SIGUSR1;

            // Clear signal_caught for next time
            signal_caught = 0;
        }
        while(restarting);
        return EXIT_SUCCESS;
    }
};

volatile sig_atomic_t runloop::signal_caught = 0;

int main(int argc, char** argv)
{
    try
    {
        // Run the runloop
        runloop r;
        return r.run(argc, argv);
    }
    catch(const std::exception& e)
    {
        // Catch-all exception handler.
        // If we've managed to get here,
        // we've encountered a fatal error
        // that couldn't be handled upstream.
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}