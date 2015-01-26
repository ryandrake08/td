#include "program.hpp"
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <cerrno> // For EINTR
#include <exception>
#include <iostream>
#include <system_error>
#include <vector>

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
#if defined(SIGUSR1)
        (void) signal(SIGUSR1, signal_handler);
#endif
#if defined(SIGUSR2)
        (void) signal(SIGUSR2, signal_handler);
#endif
        (void) signal(SIGPIPE, SIG_IGN);
    }

    ~runloop()
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
        (void) signal(SIGPIPE, SIG_DFL);
    }

    int run(int argc, const char* const argv[])
    {
        // Gather command line
        std::vector<std::string> cmdline;
        std::copy(&argv[0], &argv[argc], std::back_inserter(cmdline));

        bool restarting(false);
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
                        return EXIT_SUCCESS;
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