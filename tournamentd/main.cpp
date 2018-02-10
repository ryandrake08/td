#include "program.hpp"
#include <algorithm>
#include <clocale>
#include <cstdlib>
#include <csignal>
#include <exception>
#include <iostream>
#include <vector>

class signal_handler
{
    static void signal_handler_func(int signum)
    {
        std::signal(signum, signal_handler_func);
        signal_handler::signal_caught = signum;
    }

public:
    signal_handler()
    {
        // Initialize signal handlers
        std::signal(SIGINT, signal_handler_func);
        std::signal(SIGTERM, signal_handler_func);
#if defined(SIGUSR1)
        std::signal(SIGUSR1, signal_handler_func);
#endif
#if defined(SIGUSR2)
        std::signal(SIGUSR2, signal_handler_func);
#endif
#if defined(SIGPIPE)
        std::signal(SIGPIPE, SIG_IGN);
#endif
    }

    ~signal_handler()
    {
        // Shutdown signal handlers
        std::signal(SIGINT, SIG_DFL);
        std::signal(SIGTERM, SIG_DFL);
#if defined(SIGUSR1)
        std::signal(SIGUSR1, SIG_DFL);
#endif
#if defined(SIGUSR2)
        std::signal(SIGUSR2, SIG_DFL);
#endif
#if defined(SIGPIPE)
        std::signal(SIGPIPE, SIG_DFL);
#endif
    }

    // no copy constructors/assignment
    signal_handler(const signal_handler& other) = delete;
    signal_handler& operator=(const signal_handler& other) = delete;

    // accessor for caught signal
    volatile static std::sig_atomic_t signal_caught;
};

volatile std::sig_atomic_t signal_handler::signal_caught = 0;

int main(int argc, char** argv)
{
    // set default locale
    std::setlocale(LC_ALL, "");

    try
    {
        // signal handler for entire program
        signal_handler sig_handler;

        // gather command line
        std::vector<std::string> cmdline(&argv[0], &argv[argc]);

        bool restarting(false);
        do
        {
            restarting = false;

            program c(cmdline);

#if defined(SIGUSR2)
            while(signal_handler::signal_caught == 0 || signal_handler::signal_caught == SIGUSR2)
            {
                // notify program of SIGUSR2 so it can take some custom action (on systems that support SIGUSR2)
                if(signal_handler::signal_caught == SIGUSR2)
                {
                    if(c.sigusr2())
                    {
                        return EXIT_SUCCESS;
                    }

                    // clear signal_caught to continue calling run loop
                    signal_handler::signal_caught = 0;
                }
#else
            while(signal_handler::signal_caught == 0)
            {
#endif
                if(c.run())
                {
                    return EXIT_SUCCESS;
                }
            }

#if defined(SIGUSR1)
            // restart gracefully on SIGUSR1 (on systems that support SIGUSR1)
            if(signal_handler::signal_caught == SIGUSR1)
            {
                restarting = true;

                // clear signal_caught for next time
                signal_handler::signal_caught = 0;
            }
#endif
        }
        while(restarting);

        std::cerr << "FATAL ERROR: caught signal " << signal_handler::signal_caught << std::endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception& e)
    {
        // catch-all exception handler. if we've managed to get here, we've encountered a fatal error that couldn't be handled upstream.
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
